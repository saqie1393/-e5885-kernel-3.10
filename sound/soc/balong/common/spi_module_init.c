/*
 *  sound/soc/balong/spi_module_init.c
 *
 *  Copyright (c) 2009 Huawei Technology Co., Ltd
 *  Author: Huawei Technology Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */
 /******************************修改记录***************************************
******************************************************************************/


#include <linux/module.h>
#include <sound/soc.h>
#include <product_config.h>
#include "spi.h"

#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include "ios_pd_drv_macro.h"
#include "ios_ao_drv_macro.h"
#include "mbb_bsp_reg_def.h"

#include "bsp_sram.h"
#include "bsp_nvim.h"
#include "product_nv_def.h"
#include "product_nv_id.h"

#if (FEATURE_ON == MBB_MLOG)
#include <linux/mlog_lib.h>
#endif

//#define OPEN_SPIDEBUG           /*开启调试*/

#define SPI_TEST_NO 0         //SPI MASTER NO
#define SPI_TEST_CS 0         //SPI CS CHOICE
#define dtime              0x02

#define MAX_BUF_SIZE       (128 + 1)

#define SPI_SELECT          1
#define   NV_ID_SOFT_FACTORY_CFG  (36)

static struct class *spi_class;
/*init the major, will be change when register a char dev*/
static int spi_major = 254;

struct spidev_data {
    dev_t            devt;
    struct device    *dev;

    /* buffer is NULL unless this device is open (users > 0) */
    struct mutex    buf_lock;
    unsigned        users;
    unsigned char        *buffer;
};
struct spidev_data *spidev = NULL;

static DEFINE_MUTEX(device_list_lock);

static int spi_open(struct inode *inode, struct file *filp)
{
    int status = -ENXIO;

    /*FIND THE DEVICE*/
    mutex_lock(&device_list_lock);
    if (spidev->devt == inode->i_rdev) {
        status = 0;       /*found flag*/
    }

    if (0 == status) {
        if (!spidev->buffer) {
            //alloc buffer used after
            spidev->buffer = (unsigned char*)kmalloc(MAX_BUF_SIZE, GFP_KERNEL);
            if (!spidev->buffer) {
                printk(KERN_ERR"open/ENOMEM request failed\n");
#if ( FEATURE_ON == MBB_MLOG )
                mlog_print(MLOG_SPI, mlog_lv_factory, "%s : open/ENOMEM request failed.\n", __func__);
#endif
                status = -ENOMEM;
            }
        }
        if (0 == status) {
            spidev->users++;
            filp->private_data = spidev;
            nonseekable_open(inode, filp);
        }
    }
    else
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SPI, mlog_lv_factory, "%s : spidev: nothing for minor %d\n", __func__, iminor(inode));
#endif
        printk(KERN_ERR "spidev: nothing for minor %d\n", iminor(inode));
    }

    mutex_unlock(&device_list_lock);
    return status;
}

static int spi_release(struct inode *inode, struct file *filp)
{
    struct spidev_data *spidata;
    int status = 0;

    mutex_lock(&device_list_lock);
    spidata = filp->private_data;
    filp->private_data = NULL;

    //free buffer
    spidata->users--;
    if (!spidata->users) {
        kfree(spidata->buffer);
        spidata->buffer = NULL;
    }
    mutex_unlock(&device_list_lock);

    return status;
}

static ssize_t
spi_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct spidev_data *spidata;
    int ret = 0;
    /*for spi reciver, firstly should write a data to txfifo*/
    unsigned char cmd[2] = {0xFF,0x00};

    /* the max len of test data is 128 */
    if (count >= MAX_BUF_SIZE)
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SPI, mlog_lv_factory, "%s : the size expected read is more than max len .\n", __func__);
#endif

        return -EMSGSIZE;
    }

    printk(KERN_ERR"[TEST]receiver lenth = %d\n", count);

    spidata = filp->private_data;

    mutex_lock(&spidata->buf_lock);
    /*read data from spi master*/
    ret = slic_spi_recv(SPI_TEST_NO, SPI_TEST_CS, spidata->buffer, count, cmd, 1);
    if (SPI_OK == ret)
    {
        unsigned long missing;
#ifdef OPEN_SPIDEBUG
        int i;
        for (i = 0; i < count; i++)
        {
            printk(KERN_ERR"[TEST]receiver data is 0x%x\n", spidata->buffer[i]);
        }
#endif
        /*copy receiver data to user*/
        missing = copy_to_user(buf, spidata->buffer, count);
        if (missing == count)
        {
            count = -EFAULT;
        }
        else
        {
            count = count - missing;
        }
    }
    else
    {
        printk(KERN_ERR "[TEST]reciever the number of %d data failed, ret %d",
            count, ret);
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SPI, mlog_lv_factory, "%s : [TEST]reciever the number of %d data failed, ret %d\n", __func__, count, ret);
#endif
        count = -EFAULT;
    }

    mutex_unlock(&spidata->buf_lock);

    return count;
}

static ssize_t
spi_write(struct file *filp, const char __user *buf,
        size_t count, loff_t *f_pos)
{
    struct spidev_data *spidata;
    unsigned long       missing;
    int ret;

    /* chipselect only toggles at start or end of operation */
    if (count >= MAX_BUF_SIZE)
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SPI, mlog_lv_factory, "%s : the size expected write is more than max len .\n", __func__);
#endif

        return -EMSGSIZE;
    }

    spidata = filp->private_data;

    printk(KERN_ERR"[TEST]send lenth = %d\n", count);

    mutex_lock(&spidata->buf_lock);

    /*copy the send data from user*/
    missing = copy_from_user(spidata->buffer, buf, count);
    if (0 == missing) {
#ifdef OPEN_SPIDEBUG
        int i;
        for (i = 0; i < count; i++)
        {
            printk(KERN_ERR"[TEST]send data is 0x%x\n", spidata->buffer[i]);
        }
#endif
        /*send the data*/
        ret = slic_spi_send(SPI_TEST_NO, SPI_TEST_CS, spidata->buffer, count);
        if (SPI_OK != ret)
        {
            printk(KERN_ERR "[TEST]send the number of %d data failed, ret %d",
                count, ret);
#if ( FEATURE_ON == MBB_MLOG )
            mlog_print(MLOG_SPI, mlog_lv_factory, "%s : [TEST]send the number of %d data failed, ret %d\n", __func__, count, ret);
#endif
            count = -EFAULT;
        }
    }
    else
    {
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_SPI, mlog_lv_factory, "%s : missing some user data .\n", __func__);
#endif
        count = -EFAULT;
    }
    mutex_unlock(&spidata->buf_lock);

    return count;
}

/*spi char dev file operation, not support seek*/
static const struct file_operations spi_fops = {
    .owner = THIS_MODULE,
    .llseek = no_llseek,
    .read = spi_read,
    .write = spi_write,
    .open = spi_open,
    .release = spi_release,
};

/*lint -e124*/
/*****************************************************************************
* 函 数 名  : spi_gpio_init
*
* 功能描述  : change the gpio register to SPI fucntion
* 输入参数  : none

* 输出参数  :none
*
* 返 回 值  : none
*
* 其它说明  :
*
*****************************************************************************/
static void spi_gpio_init(void)
{
    /*spi0_clk管脚复用配置*/
    CLR_IOS_SPI0_CLK_CTRL2_2;
    SET_IOS_SPI0_CLK_CTRL2_1;
    OUTSET_IOS_PD_IOM_CTRL54;
    CLR_IOS_GPIO3_3_CTRL1_1;
    CLR_IOS_UART3_RTS_CTRL2_1;
    /*spi0_clk管脚上下拉配置*/
    PUSET_IOS_PD_IOM_CTRL54;
    /*spi0_cs0_n管脚复用配置*/
    CLR_IOS_SPI0_CS0_CTRL3_2;
    CLR_IOS_SPI0_CS0_CTRL3_3;
    SET_IOS_SPI0_CS0_CTRL3_1;
    OUTSET_IOS_PD_IOM_CTRL55;
    CLR_IOS_GPIO3_4_CTRL1_1;
    CLR_IOS_UART3_CTS_CTRL2_1;
    /*spi0_cs0_n管脚上下拉配置*/
    PUSET_IOS_PD_IOM_CTRL55;
    /*spi0_dio管脚复用配置*/
    CLR_IOS_SPI0_DIO_CTRL2_2;
    SET_IOS_SPI0_DIO_CTRL2_1;
    CLR_IOS_GPIO3_5_CTRL1_1;
    CLR_IOS_UART3_RXD_CTRL3_1;
    /*spi0_do管脚复用配置*/
    CLR_IOS_SPI0_DO_CTRL2_2;
    SET_IOS_SPI0_DO_CTRL2_1;
    OUTSET_IOS_PD_IOM_CTRL57;
    CLR_IOS_GPIO3_6_CTRL1_1;
    CLR_IOS_UART3_TXD_CTRL3_1;
}
/*lint -e124*/


/*****************************************************************************
* 函 数 名  : spi_state_get
*
* 功能描述  : check whether need to init spi driver
* 输入参数  : void

* 输出参数  :none
*
* 返 回 值  :0---uart test mode; 1---spi test mode
*
* 其它说明  :when in spi test mode, spi test driver need to be load
*
*****************************************************************************/
static int spi_state_get(void)
{
    int ret = 0;

    /*we do not judge the flag here, just read it and the return
     the caller will judge the flag, and check whether need to load
     the SPI test driver*/
    /*in factory mode*/
#if (FEATURE_ON == MBB_FACTORY)
    {
        /*factory mode: use share memery,and will lost after power down*/
        multipins_share *multipins_data = (multipins_share *)SARM_MULTIPINS_ADDR;
        if (MULTIPINS_STATUS == multipins_data->MULTIPINS_FLAG)
        {
            ret = multipins_data->UART_SPI;
        }
        printk(KERN_ERR"[SPI]share memery : %d, status: 0x%x\n",
            multipins_data->UART_SPI, multipins_data->MULTIPINS_FLAG);
    }
    /*in normal software version*/
#else
    {
        NV_MULTIPINS_TYPE  MultiPinsPara = {0};
        /*normal software: use nv to store flag and will not lost after power down*/
        if (NV_OK != NV_ReadEx(MODEM_ID_0, NV_MULTIPINS_CONFIG, &MultiPinsPara,
            sizeof(NV_MULTIPINS_TYPE)))
        {
            printk(KERN_ERR "Read NV_MULTIPINS_CONFIG Nvim Failed\n");
        }
        else
        {
            ret = MultiPinsPara.UART_SPI;
        }
        printk(KERN_ERR"[SPI]NV : %d\n",MultiPinsPara.UART_SPI);
    }
#endif

    return ret;
}


#ifdef OPEN_SPIDEBUG
static ssize_t spi_store(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t len)
{
    int i = 0;
    int ret = -1;
    unsigned char send_buf[MAX_BUF_SIZE] = {0};
    int send_len = 0;
    unsigned char rec_buf[MAX_BUF_SIZE] = {0};
    /*for spi reciver, firstly should write a data to txfifo*/
    unsigned char cmd[2] = {0xFF,0x00};
    int rec_len = 0;

    printk(KERN_ERR"[TEST] spi_store in\n");
    /*when use cmd "echo xxxx > /dev/spi0", there is a \n or \r at the end of input string
    we need to excluse the \r or \n to send*/
    send_len = min(len - 1, sizeof(send_buf) - 1);
    memcpy(send_buf, buf, send_len);

    printk(KERN_ERR "[TEST] send is %s, len = %d\n", send_buf, send_len);
    /*send the data to slave*/
    for (i = 0; i < 1; i++)
    {
        ret = slic_spi_send (SPI_TEST_NO, SPI_TEST_CS, &send_buf[i], send_len);
        if (SPI_OK != ret)
        {
            printk(KERN_ERR "[TEST]send the %d data: x%x failed, ret %d",
                i, send_buf[i], ret);
            send_len = i;
            break;
        }
    }
    /*printk the send data for debug*/
    for (i = 0; i < send_len; i++)
    {
        printk(KERN_ERR "[TEST] send_buf is 0x%2x\n", send_buf[i]);
    }
    /*receiver the data from slave
    notice: before receiver data, a byte data must write to txfifo
    the receiver len of data must equal to send lenth*/
    rec_len = send_len;
    for (i = 0; i < 1; i++)
    {
        ret = slic_spi_recv(SPI_TEST_NO, SPI_TEST_CS, &rec_buf[i], rec_len, cmd, 1);
        if (SPI_OK != ret)
        {
            printk(KERN_ERR "[TEST]reciever the %d data: 0x%x failed, ret %d",
                i, rec_buf[i], ret);
            rec_len = i;
            break;
        }
    }
    /*print the receiver data*/
    for (i = 0; i < rec_len; i++)
    {
        printk(KERN_ERR "[TEST] rec_buf is 0x%2x\n", rec_buf[i]);
    }
    /*the send_len is excluse the \r or \n, so the return size must
    bigger than send_len, to avoid the second time of system call*/
    return send_len + 1;
}
static ssize_t spi_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret = -1;
    int i = 0;
    unsigned char rec_buf[MAX_BUF_SIZE] = {0};
    /*for spi reciver, firstly should write a data to txfifo*/
    unsigned char cmd[2] = {0xFF,0x00};
    int rec_len = 1;

    printk(KERN_ERR"[TEST] spi_show in\n");
    /*begin to reciver data, only one byte receiver every time
    when in read mode, master has to produce clk , the slave will
    send the data to master*/
    for (i = 0; i < rec_len; i++)
    {
        ret = slic_spi_recv(SPI_TEST_NO, SPI_TEST_CS, &rec_buf[i], 1, cmd, 1);
        if (SPI_OK != ret)
        {
            printk(KERN_ERR "[TEST]reciever the %d data: x%x failed, ret %d",
                i, rec_buf[i], ret);
            rec_len = i;
            break;
        }
        printk(KERN_ERR "[TEST] rec_buf is 0x%2x, result = %d\n", rec_buf[i], ret);
    }

    printk(KERN_ERR "[TEST] rec_buf is : %s, len = %d\n", rec_buf, rec_len);

    return 0;
}

static DEVICE_ATTR(spi_test, S_IRUGO | S_IWUSR, spi_show, spi_store);

static struct attribute *dev_attrs[] = {
    &dev_attr_spi_test.attr,
    NULL,
};

static struct attribute_group dev_attr_grp = {
    .attrs = dev_attrs,
};
#endif

int spi_balong_init(void)
{
    int ret;

    /*check whether need to load spi test drivr, if no, then do nothing, just return*/
    if (SPI_SELECT != spi_state_get())
    {
        printk(KERN_ERR"[SPI]spi module will do nothing due to set to spi test mode\n");
        return 0;
    }

    /*spidev struct memmery alloc*/
    spidev = (struct spidev_data *)kzalloc(sizeof(*spidev), GFP_KERNEL);
    if (!spidev)
    {
        return -ENOMEM;
    }

    /*init lock will be used later*/
    mutex_init(&spidev->buf_lock);

    /*spi gpio init*/
    spi_gpio_init();
    /*spi master init*/
    ret = slic_spi_init(SPI_TEST_NO);
    printk(KERN_ERR"[TEST]spi init result: %u\n", ret);

    /*char dev register*/
    spi_major = register_chrdev(0, "spi", &spi_fops);
    spi_class = class_create(THIS_MODULE, "spi");

    if (IS_ERR(spi_class)) {
        printk(KERN_ERR "[TEST]Error creating spi class.\n");
        unregister_chrdev(spi_major, "spi");
        ret = PTR_ERR(spi_class);
        goto error_create_class;
    }

    spidev->devt = MKDEV(spi_major, 0);
    spidev->dev = device_create(spi_class, NULL, spidev->devt, NULL, "spi%d", 0);
    if (IS_ERR(spidev->dev)) {
        ret = PTR_ERR(spidev->dev);
        goto error_create_dev;
    }
#ifdef OPEN_SPIDEBUG
    /*register debugfs for debug*/
    ret = sysfs_create_group(&(spidev->dev->kobj), &dev_attr_grp);
    if (ret < 0)
    {
        printk("[TEST] Error, could not create spi test interface\n");
        goto error_create_sysfs;
    }
#endif
    printk(KERN_ERR "[TEST]spi init ok!\n");

    return 0;

#ifdef OPEN_SPIDEBUG
error_create_sysfs:
    device_destroy(spi_class, spidev->devt);
#endif
error_create_dev:
    class_destroy(spi_class);
    unregister_chrdev(spi_major, "spi");
error_create_class:
    kfree(spidev);
    spidev = NULL;
    spi_class = NULL;

    printk(KERN_ERR "[TEST]spi init error!\n");

    return ret;
}

void spi_balong_exit(void)
{
#ifdef OPEN_SPIDEBUG
    sysfs_remove_group(&(spidev->dev->kobj), &dev_attr_grp);
#endif
    device_destroy(spi_class, spidev->devt);
    class_destroy(spi_class);
    unregister_chrdev(spi_major, "spi");
    kfree(spidev);
    spidev = NULL;
    spi_class = NULL;

    return;
}

module_init(spi_balong_init);
module_exit(spi_balong_exit);


MODULE_AUTHOR("Huawei Technology Co., Ltd");
MODULE_DESCRIPTION("Balong ASoC Driver");
MODULE_LICENSE("GPL");

