#ifndef __ENMU_H__
#define __ENMU_H__

#ifdef __cplusplus
extern "C" { /* allow C++ to use these headers */
#endif /* __cplusplus */

/* usb enum done interface */
void bsp_usb_init_enum_stat(void);
void bsp_usb_host_init_enum_stat(void);
int bsp_usb_is_all_enum(int is_host);
int bsp_usb_is_all_disable(int is_host);
#define bsp_usb_add_setup_dev(intf_id) \
    bsp_usb_add_setup_dev_fdname(intf_id, __FILE__, 0)
void bsp_usb_del_setup_dev(unsigned intf_id, int is_host);
void bsp_usb_add_setup_dev_fdname(unsigned intf_id, char* fd_name, int is_host);
void bsp_usb_set_enum_stat(unsigned intf_id, int enum_stat, int is_host);
void bsp_usb_wait_cdev_created(void);

#endif    /* End of __ENMU_H__ */