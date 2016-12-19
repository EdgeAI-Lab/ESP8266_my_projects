#ifndef __FOTA_H__
#define __FOTA_H__

#define SPI_FLASH_SEC_SIZE      4096

#define USER_BIN1               0x00
#define USER_BIN2               0x01

#define UPGRADE_FLAG_IDLE       0x00
#define UPGRADE_FLAG_START      0x01
#define UPGRADE_FLAG_FINISH     0x02

#define UPGRADE_FW_BIN1         0x00
#define UPGRADE_FW_BIN2         0x01

extern void system_upgrade_init();
extern bool system_upgrade(uint8 *data, uint32 len);

extern void upgrade_recycle(void);
extern void upgrade_download(int sta_socket,char *pusrdata, unsigned short length);
extern void fota_begin(void *pvParameters);
//void fota_begin(void);



#endif