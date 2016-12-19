/*
 * Tianyuan tec
 *
 *
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "freertos/semphr.h"

/* wifi & server */
#define DEMO_WIFI_SSID "rhxtune"
#define DEMO_WIFI_PASSWORD "tianyuan"
//#define SERVER_IP "10.2.10.41"
//#define SERVER_PORT 80

#define SERVER_IP "10.2.10.45"
#define SERVER_PORT 8080

/* http header */
#define pheadbuffer "Connection: close\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.99 Safari/537.36\r\n\
Accept: */*\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"


/* Device infomation */
#define PRODUCT       "HUADI"
#define DEVICE        "HDZT00A1"
#define HARDWAREVER   "V-1-2"
#define ID            "H1ZT121603300012"
#define VERSION       "HDZT00A1-20161021-1"


/* upgrade time zone */
#define UP_HOUR       8
#define DOWN_HOUR     23

/*  Flash  */
#define SPI_FLASH_SEC_SIZE 4096
#define M                  1
#define N                  0x3E
#define NO_UPGRADE         1
#define DONE_UPGRADE       2


/* the url of bin file(new program) */
//extern char *downloadUrl;
extern char *domain;
extern char *srcPath;

extern char *pbuf;
extern char *recv_buf;
//extern char *pDstOut;

extern char *venderCode;
extern char *productCode;
extern char *hardware;


extern xTaskHandle *ota_task_handle;
extern xTaskHandle *time_task_handle;


extern xSemaphoreHandle getimeSemaphore;
extern xSemaphoreHandle fotaSemaphore;

uint8 initMem(void);
void freeMemory(void);

#endif

