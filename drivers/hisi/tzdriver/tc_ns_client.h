

#ifndef _TC_NS_CLIENT_H_
#define _TC_NS_CLIENT_H_

typedef struct {
	__u32 method;
	__u32 mdata;
} TC_NS_ClientLogin;

typedef union {
    struct {
        unsigned int buffer;
        unsigned int buffer_h_addr;
        unsigned int offset;
        unsigned int h_offset;
        unsigned int size_addr;
        unsigned int size_h_addr;
    } memref;
    struct {
        unsigned int a_addr;
        unsigned int a_h_addr;
        unsigned int b_addr;
        unsigned int b_h_addr;
    } value;
} TC_NS_ClientParam;

typedef struct {
	__u32 code;
	__u32 origin;
} TC_NS_ClientReturn;

typedef struct {
	unsigned char uuid[16];
	__u32 session_id;
	__u32 cmd_id;
	TC_NS_ClientReturn returns;
	TC_NS_ClientLogin login;
	TC_NS_ClientParam params[4];
	__u32 paramTypes;
	__u8 started;
} TC_NS_ClientContext;

typedef struct {
	uint32_t seconds;
	uint32_t millis;
} TC_NS_Time;
#ifdef CONFIG_BALONG_PLATFORM
typedef struct Client_TimeOut {
	unsigned int session_id;
	unsigned char uuid[16];
	unsigned int timeout;
} TC_Client_TimeOut;
#endif
#define	vmalloc_addr_valid(kaddr) \
	(((void *)(kaddr) >= (void *)VMALLOC_START) && \
	((void *)(kaddr) < (void *)VMALLOC_END))

#define IMG_LOAD_FIND_NO_DEV_ID  0xFFFF00A5
#define IMG_LOAD_FIND_NO_SHARE_MEM 0xFFFF00A6
#define IMG_LOAD_SECURE_RET_ERROR 0xFFFF00A7

#define TST_CMD_01 (1)
#define TST_CMD_02 (2)
#define TST_CMD_03 (3)
#define TST_CMD_04 (4)
#define TST_CMD_05 (5)

#define MAX_SHA_256_SZ 32

#define TC_NS_CLIENT_IOCTL_SES_OPEN_REQ \
	 _IOW(TC_NS_CLIENT_IOC_MAGIC, 1, TC_NS_ClientContext)
#define TC_NS_CLIENT_IOCTL_SES_CLOSE_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 2, TC_NS_ClientContext)
#define TC_NS_CLIENT_IOCTL_SEND_CMD_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 3, TC_NS_ClientContext)
#define TC_NS_CLIENT_IOCTL_SHRD_MEM_RELEASE \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 4, unsigned int)
#define TC_NS_CLIENT_IOCTL_WAIT_EVENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 5, unsigned int)
#define TC_NS_CLIENT_IOCTL_SEND_EVENT_RESPONSE \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 6, unsigned int)
#define TC_NS_CLIENT_IOCTL_REGISTER_AGENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 7, unsigned int)
#define TC_NS_CLIENT_IOCTL_UNREGISTER_AGENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 8, unsigned int)
#define TC_NS_CLIENT_IOCTL_LOAD_APP_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 9, TC_NS_ClientContext)
#define TC_NS_CLIENT_IOCTL_NEED_LOAD_APP \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 10, TC_NS_ClientContext)
#define TC_NS_CLIENT_IOCTL_ALLOC_EXCEPTING_MEM \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 12, unsigned int)
#define TC_NS_CLIENT_IOCTL_CANCEL_CMD_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 13, TC_NS_ClientContext)
#define TC_NS_CLIENT_IOCTL_LOGIN \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 14, int)
#define TC_NS_CLIENT_IOCTL_TST_CMD_REQ \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 15, int)
#define TC_NS_CLIENT_IOCTL_TUI_EVENT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 16, int)
#define TC_NS_CLIENT_IOCTL_SYC_SYS_TIME \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 17, TC_NS_Time)
#define TC_NS_CLIENT_IOCTL_SET_NATIVE_IDENTITY \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 18, int)
#ifdef CONFIG_BALONG_PLATFORM
#define TC_NS_CLIENT_IOCTL_TIMEOUT \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 19, TC_Client_TimeOut)
#define TC_NS_CLIENT_IOCTL_KILLSESSION \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 20, TC_NS_ClientContext)
#endif
#define TC_NS_CLIENT_IOCTL_LOAD_TTF_FILE \
	_IOWR(TC_NS_CLIENT_IOC_MAGIC, 21, unsigned int)
 
#endif
