/*
 * Gadget Driver for Android ADB
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include "hw_pnp_api.h"
#include "f_adb_mbb.c"
#include "usb_vendor.h"

#define ADB_BULK_BUFFER_SIZE           4096

/* number of tx requests to allocate */
#define TX_REQ_MAX 4
#define RX_REQ_MAX 2
static const char adb_shortname[] = "android_adb";
struct adb_common
{
    struct list_head    list;
    spinlock_t    lock;
};
struct adb_dev
{
    struct list_head    list;
    struct usb_function function;
    struct usb_composite_dev* cdev;
    spinlock_t lock;

    struct usb_ep* ep_in;
    struct usb_ep* ep_out;

    atomic_t online;
    atomic_t error;
    atomic_t security_port_lock;

    atomic_t read_excl;
    atomic_t write_excl;
    atomic_t open_excl;
#ifdef USB_SOLUTION
    atomic_t sleep_flag;   //标记是否已下发^PSTANDBY命令
#endif /* USB_SOLUTION */
    struct list_head tx_idle;

    wait_queue_head_t read_wq;
    wait_queue_head_t write_wq;
    struct usb_request* rx_req[RX_REQ_MAX];
    int rx_done;
    int port_num;
    int data_id;
};

struct adb_common   g_adb_comm;

struct adb_info
{
    unsigned int stat_rx_completed;
    unsigned int stat_rx_bytes;
    unsigned int stat_tx_completed;
    unsigned int stat_tx_bytes;
    unsigned int stat_open_count;
    unsigned int stat_wait_rx_done_begin;
    unsigned int stat_wait_rx_done_end;
    unsigned int stat_wait_rx_lock_begin;
    unsigned int stat_wait_rx_lock_end;
    unsigned int stat_wait_tx_begin;
    unsigned int stat_wait_tx_end;
    unsigned int stat_setup;
    unsigned int stat_cleanup;
    unsigned int stat_enum;
    int stat_security_port_lock;
};

struct  adb_info g_adb_info;

static struct usb_interface_descriptor adb_interface_desc =
{
    .bLength                = USB_DT_INTERFACE_SIZE,
    .bDescriptorType        = USB_DT_INTERFACE,
    .bInterfaceNumber       = 0,
    .bNumEndpoints          = 2,
    .bInterfaceClass        = 0xFF,
    .bInterfaceSubClass     = 0x42,
    .bInterfaceProtocol     = 1,
};

static struct usb_endpoint_descriptor adb_highspeed_in_desc =
{
    .bLength                = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType        = USB_DT_ENDPOINT,
    .bEndpointAddress       = USB_DIR_IN,
    .bmAttributes           = USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor adb_highspeed_out_desc =
{
    .bLength                = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType        = USB_DT_ENDPOINT,
    .bEndpointAddress       = USB_DIR_OUT,
    .bmAttributes           = USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor adb_fullspeed_in_desc =
{
    .bLength                = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType        = USB_DT_ENDPOINT,
    .bEndpointAddress       = USB_DIR_IN,
    .bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor adb_fullspeed_out_desc =
{
    .bLength                = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType        = USB_DT_ENDPOINT,
    .bEndpointAddress       = USB_DIR_OUT,
    .bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header* fs_adb_descs[] =
{
    (struct usb_descriptor_header*)& adb_interface_desc,
    (struct usb_descriptor_header*)& adb_fullspeed_in_desc,
    (struct usb_descriptor_header*)& adb_fullspeed_out_desc,
    NULL,
};

static struct usb_descriptor_header* hs_adb_descs[] =
{
    (struct usb_descriptor_header*)& adb_interface_desc,
    (struct usb_descriptor_header*)& adb_highspeed_in_desc,
    (struct usb_descriptor_header*)& adb_highspeed_out_desc,
    NULL,
};


/* temporary variable used between adb_open() and adb_gadget_bind() */
static struct adb_dev* _adb_dev;

static inline struct adb_dev* func_to_adb(struct usb_function* f)
{
    return container_of(f, struct adb_dev, function);
}


static struct usb_request* adb_request_new(struct usb_ep* ep, int buffer_size)
{
    struct usb_request* req = usb_ep_alloc_request(ep, GFP_KERNEL);

    if (!req)
    { return NULL; }

    /* now allocate buffers for the requests */
    req->buf = kmalloc(buffer_size, GFP_KERNEL);

    if (!req->buf)
    {
        usb_ep_free_request(ep, req);
        return NULL;
    }

    return req;
}

static void adb_request_free(struct usb_request* req, struct usb_ep* ep)
{
    if (req)
    {
        kfree(req->buf);
        usb_ep_free_request(ep, req);
    }
}

static inline int adb_lock(atomic_t* excl)
{
    if (atomic_inc_return(excl) == 1)
    {
        return 0;
    }
    else
    {
        atomic_dec(excl);
        return -1;
    }
}

static inline void adb_unlock(atomic_t* excl)
{
    atomic_dec(excl);
}

/* add a request to the tail of a list */
void adb_req_put(struct adb_dev* dev, struct list_head* head,
                 struct usb_request* req)
{
    unsigned long flags;

    spin_lock_irqsave(&dev->lock, flags);
    list_add_tail(&req->list, head);
    spin_unlock_irqrestore(&dev->lock, flags);
}

/* remove a request from the head of a list */
struct usb_request* adb_req_get(struct adb_dev* dev, struct list_head* head)
{
    unsigned long flags;
    struct usb_request* req;

    spin_lock_irqsave(&dev->lock, flags);

    if (list_empty(head))
    {
        req = 0;
    }
    else
    {
        req = list_first_entry(head, struct usb_request, list);
        list_del(&req->list);
    }

    spin_unlock_irqrestore(&dev->lock, flags);
    return req;
}

static void adb_complete_in(struct usb_ep* ep, struct usb_request* req)
{
    struct adb_dev* dev = _adb_dev;
    g_adb_info.stat_tx_completed++;
    if (req->status != 0)
    { atomic_set(&dev->error, 1); }
    g_adb_info.stat_tx_bytes += req->actual;
    adb_req_put(dev, &dev->tx_idle, req);

    wake_up(&dev->write_wq);
}

static void adb_complete_out(struct usb_ep* ep, struct usb_request* req)
{
    struct adb_dev* dev = _adb_dev;

    dev->rx_done = 1;
    g_adb_info.stat_rx_completed++;

    if (req->status != 0)
    { atomic_set(&dev->error, 1); }
    g_adb_info.stat_rx_bytes += req->actual;
    wake_up(&dev->read_wq);
}

static int adb_create_bulk_endpoints(struct adb_dev* dev,
                                     struct usb_endpoint_descriptor* in_desc,
                                     struct usb_endpoint_descriptor* out_desc)
{
    struct usb_composite_dev* cdev = dev->cdev;
    struct usb_request* req;
    struct usb_ep* ep;
    int i;

    DBG(cdev, "create_bulk_endpoints dev: %p\n", dev);

    ep = usb_ep_autoconfig(cdev->gadget, in_desc);

    if (!ep)
    {
        DBG(cdev, "usb_ep_autoconfig for ep_in failed\n");
        return -ENODEV;
    }

    DBG(cdev, "usb_ep_autoconfig for ep_in got %s\n", ep->name);
    ep->driver_data = dev;		/* claim the endpoint */
    dev->ep_in = ep;

    ep = usb_ep_autoconfig(cdev->gadget, out_desc);

    if (!ep)
    {
        DBG(cdev, "usb_ep_autoconfig for ep_out failed\n");
        return -ENODEV;
    }

    DBG(cdev, "usb_ep_autoconfig for adb ep_out got %s\n", ep->name);
    ep->driver_data = dev;		/* claim the endpoint */
    dev->ep_out = ep;

    for (i = 0; i < TX_REQ_MAX; i++)
    {
        req = adb_request_new(dev->ep_in, ADB_BULK_BUFFER_SIZE);

        if (!req)
        { goto fail_TX; }

        req->complete = adb_complete_in;
        adb_req_put(dev, &dev->tx_idle, req);
    }

    /* now allocate requests for our endpoints */
    for (i = 0; i < RX_REQ_MAX; i++)
    {
        req = adb_request_new(dev->ep_out, ADB_BULK_BUFFER_SIZE);

        if (!req)
        { goto fail_RX; }

        req->complete = adb_complete_out;
        dev->rx_req[i] = req;
    }

    return 0;

fail_RX :

    while ((req = adb_req_get(dev, &dev->tx_idle)))
    { adb_request_free(req, dev->ep_in); }

fail_TX :

    for (i = 0; i < RX_REQ_MAX; i++)
    {
        adb_request_free(dev->rx_req[i], dev->ep_out);
    }

    printk(KERN_ERR "adb_bind() could not allocate requests\n");
    return -1;
}

#ifdef USB_SECURITY
void adb_notify_to_work(void)
{
    struct adb_dev* dev = _adb_dev;

    atomic_set(&dev->security_port_lock, 0);
    wake_up(&dev->read_wq);

}
#endif/*USB_SECURITY*/

static ssize_t adb_read(struct file* fp, char __user* buf,
                        size_t count, loff_t* pos)
{
    struct adb_dev* dev = _adb_dev;//fp->private_data;
    struct usb_request* req;
    int r = count, xfer;
    int ret, len;
    DBG_I(MBB_ADB,"adb_read(%d)\n", count);

    if (!_adb_dev)
    { return -ENODEV; }

    if (count > ADB_BULK_BUFFER_SIZE)
    { return -EINVAL; }

#ifdef USB_SOLUTION
    /* 如果已经下发^PSTANDBY则不允许通过adb端口读数据 */
    if (0 == atomic_read(&dev->sleep_flag))
    {
        return -ENODEV;
    }
#endif /* USB_SOLUTION */

#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 判断adb online是否在线 */
    if (0 == atomic_read(&_adb_dev->online))
    {
        return -EBUSY;
    }
#endif

    if (adb_lock(&dev->read_excl))
    { return -EBUSY; }

    if (NULL == buf)
    {
        adb_unlock(&dev->read_excl);
        return -EINVAL; 
    }

    /* we will block until we're online and unlocked */
    while (atomic_read(&dev->security_port_lock) ||
		!(atomic_read(&dev->online) || atomic_read(&dev->error)))
    {
        DBG_I(MBB_ADB,"adb_read: waiting for online state\n");
        g_adb_info.stat_wait_rx_lock_begin++;
        ret = wait_event_interruptible(dev->read_wq,
                                       (!atomic_read(&dev->security_port_lock))
                                       && (atomic_read(&dev->online) || atomic_read(&dev->error)));

        g_adb_info.stat_wait_rx_lock_end++;
        if (ret < 0)
        {
            adb_unlock(&dev->read_excl);
            return ret;
        }
    }

    if (atomic_read(&dev->error))
    {
        r = -EIO;
        goto done;
    }

requeue_req:
    len = ALIGN(count, dev->ep_out->maxpacket);

    /* queue a request */
    req = dev->rx_req[0];
    req->length = len;
    dev->rx_done = 0;
    ret = usb_ep_queue(dev->ep_out, req, GFP_KERNEL);

    if (ret < 0)
    {
        DBG_I(MBB_ADB,"adb_read: failed to queue req %p (%d)\n", req, ret);
        r = -EIO;
        atomic_set(&dev->error, 1);
        goto done;
    }
    else
    {
        DBG_I(MBB_ADB,"rx %p queue\n", req);
    }

    g_adb_info.stat_wait_rx_done_begin++;
    /* wait for a request to complete */
    ret = wait_event_interruptible(dev->read_wq, dev->rx_done);
    g_adb_info.stat_wait_rx_done_end++;

    if (ret < 0)
    {
        atomic_set(&dev->error, 1);
        r = ret;
        usb_ep_dequeue(dev->ep_out, req);
        goto done;
    }

    if (!atomic_read(&dev->error))
    {
        /* If we got a 0-len packet, throw it back and try again. */
        if (req->actual == 0)
        { goto requeue_req; }

        DBG_I(MBB_ADB,"rx %p %d\n", req, req->actual);
        xfer = (req->actual < count) ? req->actual : count;
        r = xfer;
#ifdef BSP_CONFIG_BOARD_TELEMATIC
        if (0 == get_ports_sec_lock())
        {
            if (copy_to_user(buf, req->buf, xfer))
            {
                r = -EFAULT;
            }
        }
        else
        {
            r = -EACCES;
        }
#else
        if (copy_to_user(buf, req->buf, xfer))
        {
            r = -EFAULT;
        }
#endif
    }
    else
    { r = -EIO; }

done:
    adb_unlock(&dev->read_excl);
    DBG_I(MBB_ADB,"adb_read returning %d\n", r);
    return r;
}

static ssize_t adb_write(struct file* fp, const char __user* buf,
                         size_t count, loff_t* pos)
{
    struct adb_dev* dev = _adb_dev;//fp->private_data;
    struct usb_request* req = 0;
    int r = count, xfer;
    int ret;

    if (!_adb_dev)
    {
        pr_err("%s:the _adb_dev is NULL!r\n",__FUNCTION__);
        return -ENODEV; 
    }

    DBG_I(MBB_ADB,"adb_write(%d)\n", count);
#ifdef BSP_CONFIG_BOARD_TELEMATIC
    if (get_ports_sec_lock())
    {
        return -EACCES;
    }
#endif
    if (adb_lock(&dev->write_excl))
    { return -EBUSY; }

    while (count > 0)
    {
        if (atomic_read(&dev->error))
        {
            DBG_I(MBB_ADB,"adb_write dev->error\n");
            r = -EIO;
            break;
        }

        /* get an idle tx request to use */
        req = 0;
        g_adb_info.stat_wait_tx_begin++;
        ret = wait_event_interruptible(dev->write_wq,
                                       ((req = adb_req_get(dev, &dev->tx_idle)) ||
                                        atomic_read(&dev->error)));

        g_adb_info.stat_wait_tx_end++;
        if (ret < 0)
        {
            r = ret;
            break;
        }

        if (req != 0)
        {
            if (count > ADB_BULK_BUFFER_SIZE)
            { xfer = ADB_BULK_BUFFER_SIZE; }
            else
            { xfer = count; }

            if (copy_from_user(req->buf, buf, xfer))
            {
                r = -EFAULT;
                break;
            }

            req->length = xfer;
            ret = usb_ep_queue(dev->ep_in, req, GFP_ATOMIC);

            if (ret < 0)
            {
                DBG_I(MBB_ADB,"adb_write: xfer error %d\n", ret);
                atomic_set(&dev->error, 1);
                r = -EIO;
                break;
            }

            buf += xfer;
            count -= xfer;

            /* zero this so we don't try to free it on error exit */
            req = 0;
        }
    }

    if (req)
    { adb_req_put(dev, &dev->tx_idle, req); }

    adb_unlock(&dev->write_excl);
    DBG_I(MBB_ADB,"adb_write returning %d\n", r);
    return r;
}

static int adb_open(struct inode* ip, struct file* fp)
{
    printk(KERN_INFO "adb_open\n");

    if (!_adb_dev)
    { return -ENODEV; }

#ifdef USB_SOLUTION
    /* 如果已经下发^PSTANDBY则不允许打开adb端口 */
    if (0 == atomic_read(&_adb_dev->sleep_flag))
    {
        return -ENODEV;
    }
#endif /* USB_SOLUTION */

#ifdef BSP_CONFIG_BOARD_TELEMATIC
    /* 判断adb online是否在线 */
    if (0 == atomic_read(&_adb_dev->online))
    {
        return -EBUSY;
    }
#endif

    if (adb_lock(&_adb_dev->open_excl))
    { return -EBUSY; }

    fp->private_data = _adb_dev;
    g_adb_info.stat_open_count++;

    /* clear the error latch */
    atomic_set(&_adb_dev->error, 0);

    return 0;
}

static int adb_release(struct inode* ip, struct file* fp)
{
    printk(KERN_INFO "adb_release\n");
    if(NULL != _adb_dev)
    {
        adb_unlock(&_adb_dev->open_excl);
    }
    return 0;
}

/* file operations for ADB device /dev/android_adb */
static struct file_operations adb_fops =
{
    .owner = THIS_MODULE,
    .read = adb_read,
    .write = adb_write,
    .open = adb_open,
    .release = adb_release,
};

static struct miscdevice adb_device =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = adb_shortname,
    .fops = &adb_fops,
};




static int
adb_function_bind(struct usb_configuration* c, struct usb_function* f)
{
    struct usb_composite_dev* cdev = c->cdev;
    struct adb_dev*	dev = func_to_adb(f);
    int			id;
    int			ret;

    dev->cdev = cdev;
    DBG(cdev, "adb_function_bind dev: %p\n", dev);

    /* allocate interface ID(s) */
    id = usb_interface_id(c, f);

    if (id < 0)
    { return id; }

    adb_interface_desc.bInterfaceNumber = id;
    dev->data_id = id;
    bsp_usb_add_setup_dev((unsigned)dev->data_id);

    /* allocate endpoints */
    ret = adb_create_bulk_endpoints(dev, &adb_fullspeed_in_desc,
                                    &adb_fullspeed_out_desc);

    if (ret)
    { return ret; }

    /* support high speed hardware */
    if (gadget_is_dualspeed(c->cdev->gadget))
    {
        adb_highspeed_in_desc.bEndpointAddress =
            adb_fullspeed_in_desc.bEndpointAddress;
        adb_highspeed_out_desc.bEndpointAddress =
            adb_fullspeed_out_desc.bEndpointAddress;
    }

    dev->function.fs_descriptors = usb_copy_descriptors(fs_adb_descs);
    dev->function.hs_descriptors = usb_copy_descriptors(hs_adb_descs);

    DBG(cdev, "%s speed %s: IN/%s, OUT/%s\n",
        gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
        f->name, dev->ep_in->name, dev->ep_out->name);

    return 0;
}

static void
adb_function_unbind(struct usb_configuration* c, struct usb_function* f)
{
    struct adb_dev*	dev = func_to_adb(f);
    struct usb_request* req;
    int i;


    atomic_set(&dev->online, 0);
    atomic_set(&dev->error, 1);

    wake_up(&dev->read_wq);

    usb_free_descriptors(f->hs_descriptors);
    usb_free_descriptors(f->fs_descriptors);

    for (i = 0; i < RX_REQ_MAX; i++)
    {
        adb_request_free(dev->rx_req[i], dev->ep_out);
    }

    while ((req = adb_req_get(dev, &dev->tx_idle)))
    { adb_request_free(req, dev->ep_in); }
}

#ifdef USB_SOLUTION
/*****************************************************************************
 函 数 名  : adb_pstandby_set_disable
 功能描述  : 下发^PSTANDBY命令后置adb不可用
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :
*****************************************************************************/
void adb_pstandby_set_disable(void)
{
    struct adb_dev *dev = _adb_dev;
    if (NULL == dev)
    {
        return;
    }

    atomic_set(&dev->sleep_flag, 0);
    return;
}
#endif /* USB_SOLUTION */

static int adb_function_set_alt(struct usb_function* f,
                                unsigned intf, unsigned alt)
{
    struct adb_dev*	dev = func_to_adb(f);
    struct usb_configuration  *config = f->config;
    struct usb_composite_dev* cdev = f->config->cdev;
    unsigned long flags;
    struct adb_dev *dev_item,*next;
    int ret;

    spin_lock_irqsave(&dev->lock, flags);
    _adb_dev = NULL;
    list_for_each_entry_safe(dev_item,next,&(g_adb_comm.list),list)
    {
        if(config->bConfigurationValue == dev_item->port_num)
        {
            _adb_dev = dev_item;
        }
    }
    spin_unlock_irqrestore(&dev->lock, flags);

    DBG(cdev, "adb_function_set_alt intf: %d alt: %d\n", intf, alt);

    ret = config_ep_by_speed(cdev->gadget, f, dev->ep_in);

    if (ret)
    {
        dev->ep_in->desc = NULL;
        ERROR(cdev, "config_ep_by_speed failes for ep %s, result %d\n",
              dev->ep_in->name, ret);
        return ret;
    }

    ret = usb_ep_enable(dev->ep_in);

    if (ret)
    {
        ERROR(cdev, "failed to enable ep %s, result %d\n",
              dev->ep_in->name, ret);
        return ret;
    }

    ret = config_ep_by_speed(cdev->gadget, f, dev->ep_out);

    if (ret)
    {
        dev->ep_out->desc = NULL;
        ERROR(cdev, "config_ep_by_speed failes for ep %s, result %d\n",
              dev->ep_out->name, ret);
        usb_ep_disable(dev->ep_in);
        return ret;
    }

    ret = usb_ep_enable(dev->ep_out);

    if (ret)
    {
        ERROR(cdev, "failed to enable ep %s, result %d\n",
              dev->ep_out->name, ret);
        usb_ep_disable(dev->ep_in);
        return ret;
    }

    atomic_set(&dev->online, 1);
    bsp_usb_set_enum_stat(dev->data_id,1);

    /* readers may be blocked waiting for us to go online */
    wake_up(&dev->read_wq);
    g_adb_info.stat_enum++;
    return 0;
}

static void adb_function_disable(struct usb_function* f)
{
    struct adb_dev*	dev = func_to_adb(f);
    struct usb_composite_dev*	cdev = dev->cdev;

    DBG(cdev, "adb_function_disable cdev %p\n", cdev);
    atomic_set(&dev->online, 0);
    atomic_set(&dev->error, 1);
    usb_ep_disable(dev->ep_in);
    usb_ep_disable(dev->ep_out);
    bsp_usb_set_enum_stat(dev->data_id,0);

    /* readers may be blocked waiting for us to go online */
    wake_up(&dev->read_wq);
    g_adb_info.stat_enum = 0;
    VDBG(cdev, "%s disabled\n", dev->function.name);
}

static int adb_bind_config(struct usb_configuration* c)
{
    struct adb_dev   *dev = NULL;
    unsigned long flags;
    int port_lock = 0;

    printk(KERN_INFO "adb_bind_config\n");

    dev = kzalloc(sizeof *dev, GFP_KERNEL);

    if (!dev)
    { return -ENOMEM; }

    spin_lock_init(&dev->lock);

    spin_lock_irqsave(&dev->lock, flags);
    list_add_tail(&(dev->list),&(g_adb_comm.list));
    spin_unlock_irqrestore(&dev->lock, flags);

    init_waitqueue_head(&dev->read_wq);
    init_waitqueue_head(&dev->write_wq);

    atomic_set(&dev->open_excl, 0);
    atomic_set(&dev->read_excl, 0);
    atomic_set(&dev->write_excl, 0);

#ifdef USB_SOLUTION
    /* 初始化置为1，下发^PSTANDBY后置为0 */
    atomic_set(&dev->sleep_flag, 1);
#endif /* USB_SOLUTION */

    port_lock = usb_port_security();
    if ( port_lock ) 
    {
        /*根据产品要求，release版本不编译adb，debug版本默认adb可用，不加锁*/
        atomic_set(&dev->security_port_lock, 0);
    }
    else
    {
        atomic_set(&dev->security_port_lock, 0);
    }

    INIT_LIST_HEAD(&dev->tx_idle);

    dev->cdev = c->cdev;
    dev->function.name = "adb";
    //dev->function.fs_descriptors = fs_adb_descs;
    //dev->function.hs_descriptors = hs_adb_descs;
    dev->function.bind = adb_function_bind;
    dev->function.unbind = adb_function_unbind;
    dev->function.set_alt = adb_function_set_alt;
    dev->function.disable = adb_function_disable;
    dev->port_num = c->bConfigurationValue;

    return usb_add_function(c, &dev->function);
}

static int adb_setup(void)
{
    int ret;

    INIT_LIST_HEAD(&(g_adb_comm.list));
    spin_lock_init(&(g_adb_comm.lock));

    ret = misc_register(&adb_device);

    if (ret)
    { goto err; }

    g_adb_info.stat_setup++;
    return 0;
err:
    printk(KERN_ERR "adb gadget driver failed to initialize\n");
    return ret;
}

static void adb_cleanup(void)
{
    struct adb_dev *dev_item,*next;
    unsigned long flags;

    misc_deregister(&adb_device);
    spin_lock_irqsave(&(g_adb_comm.lock), flags);
    list_for_each_entry_safe(dev_item,next,&(g_adb_comm.list),list)
    {
       list_del(&dev_item->list);
       kfree(dev_item);
    }
    _adb_dev = NULL;
    spin_unlock_irqrestore(&(g_adb_comm.lock), flags);
    g_adb_info.stat_cleanup++;
    memset(&g_adb_info,0,sizeof(struct adb_info));
}

void test_adb_write(unsigned int len)
{
    struct adb_dev* dev = _adb_dev;
    struct usb_request* req;
    int ret = -1;
    len = (len > (dev->ep_in->maxpacket))?(dev->ep_in->maxpacket):len;
    /* queue a request */
    req = adb_req_get(dev, &dev->tx_idle);
    if(NULL == req)
    {
       printk("NULL == req test_write\n");
       return;
    }
    memset(req->buf, 'c', len); 
    req->length = len;
    
    ret = usb_ep_queue(dev->ep_in, req, GFP_ATOMIC);

    if (ret < 0)
    {
        DBG_I(MBB_ADB,"adb_write failed to queue req %p (%d)\n", req, ret);/*lint !e516*/
 
    }

}
void adb_dump()
{
    pr_emerg("===== dump stat adb info =====\n");
    pr_emerg("build version:            %s\n", __VERSION__);
    pr_emerg("build date:               %s\n", __DATE__);
    pr_emerg("build time:               %s\n", __TIME__);
    pr_emerg("stat_setup:               %d\n", g_adb_info.stat_setup);
    pr_emerg("stat_cleanup:             %d\n", g_adb_info.stat_cleanup);
    pr_emerg("stat_rx_completed:        %d\n", g_adb_info.stat_rx_completed);
    pr_emerg("stat_rx_bytes:            %d\n", g_adb_info.stat_rx_bytes);
    pr_emerg("stat_tx_completed:        %d\n", g_adb_info.stat_tx_completed);
    pr_emerg("stat_tx_bytes:            %d\n", g_adb_info.stat_tx_bytes);
    pr_emerg("stat_open_count:          %d\n", g_adb_info.stat_open_count);
    pr_emerg("stat_wait_rx_done_begin:  %d\n", g_adb_info.stat_wait_rx_done_begin);
    pr_emerg("stat_wait_rx_done_end:    %d\n", g_adb_info.stat_wait_rx_done_end);
    pr_emerg("stat_wait_rx_lock_begin:  %d\n", g_adb_info.stat_wait_rx_lock_begin);
    pr_emerg("stat_wait_rx_lock_end:    %d\n", g_adb_info.stat_wait_rx_lock_end);
    pr_emerg("stat_wait_tx_begin:       %d\n", g_adb_info.stat_wait_tx_begin);
    pr_emerg("stat_wait_tx_end:         %d\n", g_adb_info.stat_wait_tx_end);
    pr_emerg("stat_security_port_lock:  %d\n", g_adb_info.stat_security_port_lock);
    pr_emerg("stat_enum:                %d\n", g_adb_info.stat_enum);
}

