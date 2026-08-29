#ifndef _SPI_NOR_FLASH_H_
#define _SPI_NOR_FLASH_H_
#define hisi_spi_log(fmt, ...) printk(KERN_ERR"[HISI_SPI]: <%s> line = %d  "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define SPI_MAX_DELAY_TIMES 0x10000
/* W25Q128 SPI NOR Flash CMD */
#define SPI_DEV_CMD_WE      0x06    /*  */
#define SPI_DEV_CMD_VSRWE   0x50    /*  */
#define SPI_DEV_CMD_WD      0x04    /*  */
#define SPI_DEV_CMD_RSR1    0x05    /*  */
#define SPI_DEV_CMD_WSR1    0x01    /*  */
#define SPI_DEV_CMD_RSR2    0x35    /*  */
#define SPI_DEV_CMD_WSR2    0x31    /*  */
#define SPI_DEV_CMD_RSR3    0x15    /*  */
#define SPI_DEV_CMD_WSR3    0x11    /*  */
#define SPI_DEV_CMD_CE      0xC7    /* or 0x60 */
#define SPI_DEV_CMD_EPS     0x75    /*  */
#define SPI_DEV_CMD_EPR     0x7A    /*  */
#define SPI_DEV_CMD_PD      0xB9    /*  */
#define SPI_DEV_CMD_RPDID   0xAB    /*  */
#define SPI_DEV_CMD_MDID    0x90    /*  */
#define SPI_DEV_CMD_JDCID   0x9F    /*  */
#define SPI_DEV_CMD_GBL     0x7E    /*  */
#define SPI_DEV_CMD_GBU     0x98    /*  */
#define SPI_DEV_CMD_EQPIM   0x38    /*  */
#define SPI_DEV_CMD_ER      0x66    /*  */
#define SPI_DEV_CMD_RSTDEV  0x99    /*  */
#define SPI_DEV_CMD_RUID    0x4B    /*  */
#define SPI_DEV_CMD_PP      0x02    /*  */
#define SPI_DEV_CMD_QPP     0x32    /*  */
#define SPI_DEV_CMD_SE      0x20    /*  */
#define SPI_DEV_CMD_BE32    0x52    /*  */
#define SPI_DEV_CMD_BE64    0xD8    /*  */
#define SPI_DEV_CMD_RDDAT   0x03    /*  */
#define SPI_DEV_CMD_FR      0x0B    /*  */
#define SPI_DEV_CMD_FRDO    0x3B    /*  */
#define SPI_DEV_CMD_FRQO    0x6B    /*  */
#define SPI_DEV_CMD_ESR     0x44    /*  */
#define SPI_DEV_CMD_PSR     0x42    /*  */
#define SPI_DEV_CMD_RSR     0x48    /*  */
#define SPI_DEV_CMD_IBL     0x36    /*  */
#define SPI_DEV_CMD_IBU     0x39    /*  */
#define SPI_DEV_CMD_RBL     0x3D    /*  */
#define SPI_DEV_CMD_FRDIO   0xBB    /*  */
#define SPI_DEV_CMD_MDIDDIO 0x92    /*  */
#define SPI_DEV_CMD_SBW     0x77    /*  */
#define SPI_DEV_CMD_FRQIO   0xEB    /*  */
#define SPI_DEV_CMD_WRQIO   0xE7    /*  */
#define SPI_DEV_CMD_OWRQIO  0xE3    /*  */
#define SPI_DEV_CMD_MDIDQIO 0x94    /*  */

/* W25Q128 SPI NOR Flash status */
#define SPI_DEV_STATUS_BUSY     (1<<0)  /*  */
#define SPI_DEV_STATUS_WEL      (1<<1)  /*  */
#define SPI_DEV_STATUS_BP0      (1<<2)  /*  */
#define SPI_DEV_STATUS_BP1      (1<<3)  /*  */
#define SPI_DEV_STATUS_BP2      (1<<4)  /*  */
#define SPI_DEV_STATUS_TB       (1<<5)  /*  */
#define SPI_DEV_STATUS_SEC      (1<<6)  /*  */
#define SPI_DEV_STATUS_SRP0     (1<<7)  /*  */

#endif

