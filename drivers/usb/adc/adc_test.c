
#include <mdrv.h>
#include <linux/fs.h>
#include <linux/delay.h>

#define ADC_WRITE_SIZE      7680
//#define ADC_FILP_WRITE

typedef struct adc_write_info{
    ADC_BUF_INFO_STRUCT write_info;
    struct list_head list;
}adc_write_info_struct;

struct adc_test_ctx{
    UDI_HANDLE handle;
    ADC_BUF_INFO_STRUCT read_info;

    unsigned int write_size;
    unsigned int write_err;
    unsigned int write_success;
    unsigned int write_cb_called;
    unsigned int write_buf_alloc;
    unsigned int write_buf_free;
    unsigned int write_blocked;
    unsigned int read_cb_called;
    unsigned int write_wait_fail;
    unsigned int write_complete;

    wait_queue_head_t write_wait;

    struct list_head writing_queue;
    struct file *filp;
    unsigned int pos;
};

struct adc_test_ctx g_adc_test_ctx = {0};

void adc_test_write(adc_write_info_struct *write_buf)
{
    int result;

    if(write_buf->write_info.u32Size){
        result = mdrv_udi_write(g_adc_test_ctx.handle, write_buf->write_info.pVirAddr,
            write_buf->write_info.u32Size);

        if(result < 0){
            //printk("adc test write err!!!\n");
            g_adc_test_ctx.write_err++;
            g_adc_test_ctx.write_buf_free++;
            kfree(write_buf->write_info.pVirAddr);
            kfree(write_buf);

        }else{
            g_adc_test_ctx.write_success++;
        }
    }else{
        g_adc_test_ctx.write_blocked++;
    }

    return;
}

void adc_test_read_cb(void)
{
    adc_write_info_struct *write_buf;

    g_adc_test_ctx.read_cb_called++;
    mdrv_udi_ioctl(g_adc_test_ctx.handle, UDI_ADC_IOCTL_GET_READ_BUFFER_CB,
        &g_adc_test_ctx.read_info);

    //printk("[adc_test_read_cb]read size : %d \n", g_adc_test_ctx.read_info.u32Size);
#ifdef ADC_FILP_WRITE
    if(g_adc_test_ctx.filp){
        size = kernel_write(g_adc_test_ctx.filp, g_adc_test_ctx.read_info.pVirAddr,
            g_adc_test_ctx.read_info.u32Size, g_adc_test_ctx.pos);
        //printk("[kernel_write]size : %d  pos %d \n", size, g_adc_test_ctx.pos);
        g_adc_test_ctx.pos += size;
        //g_adc_test_ctx.pos += g_adc_test_ctx.read_info.u32Size;
    }
#endif

    g_adc_test_ctx.write_buf_alloc++;
    write_buf = kmalloc(sizeof(adc_write_info_struct), GFP_KERNEL);
    write_buf->write_info.pVirAddr = kmalloc(g_adc_test_ctx.read_info.u32Size, GFP_KERNEL);
    memcpy(write_buf->write_info.pVirAddr, g_adc_test_ctx.read_info.pVirAddr,
            g_adc_test_ctx.read_info.u32Size);

    write_buf->write_info.u32Size = g_adc_test_ctx.read_info.u32Size;
    mdrv_udi_ioctl(g_adc_test_ctx.handle, UDI_ADC_IOCTL_RETUR_BUFFER_CB, NULL);

    list_add_tail(&write_buf->list, &g_adc_test_ctx.writing_queue);
    adc_test_write(write_buf);


    return;
}

void adc_test_write_cb(void *buf)
{
    adc_write_info_struct *cur, *next;

    g_adc_test_ctx.write_cb_called++;
    list_for_each_entry_safe(cur, next, &g_adc_test_ctx.writing_queue, list){
        if(cur->write_info.pVirAddr == buf){
            g_adc_test_ctx.write_buf_free++;
            list_del(&cur->list);
            kfree(cur->write_info.pVirAddr);
            kfree(cur);
            return;
        }
    }
}

#ifdef ADC_FILP_WRITE
void adc_test_write_only_cb(void *buf)
{
    //wakeup
    g_adc_test_ctx.write_complete = 1;
    wake_up_interruptible(&g_adc_test_ctx.write_wait);
}

void adc_test_write_size_set(unsigned int val)
{
    g_adc_test_ctx.write_size = val;
}

void adc_test_write_only(void)
{
    int size;
    char *buf;
    int status;
    unsigned long pos = 0;
    struct file *filp;

    buf = kmalloc(g_adc_test_ctx.write_size, GFP_KERNEL);
    if(!buf){
        printk("[adc_test_write_only]buf alloc failed\n");
        return;
    }

    filp = filp_open("/data/adc2", O_RDWR | O_CREAT | O_LARGEFILE, 0);
    if(!filp){
        printk("adc_test_init: filp 2 open failed\n");
        kfree(buf);
        return;
    }

    while(1){
        size = kernel_read(filp, pos, buf, g_adc_test_ctx.write_size);
        if(size <= 0){
            printk("[adc_test_write_only]pos : %ld\n", pos);
            break;
        }

        mdrv_udi_write(g_adc_test_ctx.handle, buf, size);
        pos += size;
        g_adc_test_ctx.write_complete = 0;

        //wait
        status = wait_event_interruptible(g_adc_test_ctx.write_wait, g_adc_test_ctx.write_complete == 1);
        if (status) {
            g_adc_test_ctx.write_wait_fail++;
            printk("[adc_test_write_only] write_wait_fail \n");
            break;
        }
    }

    filp_close(filp, NULL);
    kfree(buf);
}
#endif

void adc_test_init(void)
{
    UDI_OPEN_PARAM_S                    adc_para;

    adc_para.devid            = UDI_ADC_SND0_ID;
    g_adc_test_ctx.handle = mdrv_udi_open(&adc_para);

    if (!g_adc_test_ctx.handle){
     printk("adc_test_init: Open ADC failed!\n");
     return;
    }

    INIT_LIST_HEAD(&g_adc_test_ctx.writing_queue);

#ifdef ADC_FILP_WRITE
    mdrv_udi_ioctl(g_adc_test_ctx.handle, UDI_ADC_IOCTL_SET_WRITE_CB,

         adc_test_write_only_cb);
#else
    mdrv_udi_ioctl(g_adc_test_ctx.handle, UDI_ADC_IOCTL_SET_WRITE_CB,

         adc_test_write_cb);
#endif

     mdrv_udi_ioctl(g_adc_test_ctx.handle, UDI_ADC_IOCTL_SET_READ_CB,

         adc_test_read_cb);

#ifdef ADC_FILP_WRITE
     g_adc_test_ctx.filp = filp_open("/data/adc", O_RDWR | O_CREAT | O_LARGEFILE, 0);
     if(!g_adc_test_ctx.filp){
        printk("adc_test_init: filp open failed\n");
     }
#endif

     g_adc_test_ctx.write_size = ADC_WRITE_SIZE;

     init_waitqueue_head(&g_adc_test_ctx.write_wait);

     return;

}

#ifdef ADC_FILP_WRITE
void adc_channel_switch(void)
{
    int size;
    struct file *filp;
    char *buf;
    unsigned long pos = 0;

    buf = kmalloc(2, GFP_KERNEL);
    g_adc_test_ctx.filp = filp_open("/data/adc", O_RDWR | O_LARGEFILE, 0);
    if(!g_adc_test_ctx.filp){
        printk("adc_test_init: filp 1 open failed\n");
        kfree(buf);
        return;
    }

    filp = filp_open("/data/adc2", O_RDWR | O_CREAT | O_LARGEFILE, 0);
    if(!filp){
        printk("adc_test_init: filp 2 open failed\n");
        kfree(buf);
        return;
    }

    while(1){
        size = kernel_read(g_adc_test_ctx.filp, pos, buf, 2);
        if(size <= 0){
            printk("pos : %ld\n", pos);
            break;
        }

        size = kernel_write(filp, buf, 2, pos*2);

        //printk("size:%d  filp:0x%x\n", size, g_adc_test_ctx.filp);
        size = kernel_write(filp, buf, 2, (pos*2)+2);
        //printk("size:%d  filp:0x%x\n", size, g_adc_test_ctx.filp);
        pos += 2;
    }
    filp_close(g_adc_test_ctx.filp, NULL);
    filp_close(filp, NULL);

}
#endif


