/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "esp_common.h"
#include "esp8266.h"
#include "flash.h"
#include "freertos/semphr.h"

#define DEMO_WIFI_SSID "rhxtune"
#define DEMO_WIFI_PASSWORD "tianyuan"

#define HEAD_FIRST    0
#define HEAD_SECOND   1
#define ENDPOINT_ID   2
#define CLUSTER_ID    3 // 2 bytes
#define CMD           5
#define ATTR_ID       6 // 2 bytes
#define LENGTH        8
#define DATA          9


// CMD
#define READ   1
#define WRITE  2
#define ACK    3

// Endpoint
#define ESP8266 1
#define FLASH   2

// ESP8266 Cluster
#define ESP8266_INFO 0x0101
#define ESP8266_WIFI 0x0102

// FLASH Cluster
#define FLASH_INFO 0x0101

// ESP8266 Attribute
#define CHIP_ID   0x0101
#define SDK_VER   0x0102

#define WIFI_SSID        0x0101
#define WIFI_PASSWORD    0x0102
#define WIFI_CONNECT_STA 0x0103

// Flash Attribute
#define FLASH_LISENCE  0x0101
#define FLASH_VEND_ID  0x0102
#define FLASH_PROC_ID  0x0103
#define FLASH_HARDWARE 0x0104


extern xSemaphoreHandle timerSemaphore;
extern uint32  frameStartTime;

void timer_task(void *pvParameters);

extern bool pkgFlag;
extern uint8 uart_rx_buf[40];

void data_handle(uint8 length);
unsigned char get_check_sum(unsigned char *pack, unsigned short pack_len);



#endif

