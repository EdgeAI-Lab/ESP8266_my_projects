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

#include "esp_common.h"


/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


/**********************************OUR CODE*******************************************
*How to Use this RTOS SDK example 
uart_init_new(); Initalizes UART0 & UART1 of ESP8266 (You can change the Baud rates & Structure of the UART frame here)
***UART1 only has a single Tx Pin. There is no Receiver at UART1

uart0_tx_buffer(str pointer, length); Sends the Data if the UART Ports are Initialized 
Changes made in the Library : Changed the Interrupt Handler Function : uart0_rx_intr_handler for Receiving Data in data buffer 

*************************************************************************************/

#include "uart.h"

// object
// ESP8266
#define CHIP_ID	     0x10
#define SDK_VERSION	 0x11
#define AP_MAC	     0x12
#define STA_MAC	     0x13
#define VENDCODE	 0x14
#define DEVICE	     0x15
#define HARDWAREVER	 0x16
#define ID	         0x17
#define VERSION	     0x18

// Flash
#define LISENCE      0x20

bool pkgFlag = FALSE;
uint8 dataBuf[40];


uint16 crc16(uint8 *puchMsg,uint8 crc_count)
{
  uint8 i ,j;
  uint8 XORResult;
  uint16 xorCRC = 0xA001; //计算CRC的多项式
  uint16 CRC = 0xFFFF;
  for ( i = 0; i <crc_count; i++)
  {
    CRC ^= puchMsg[i];
    for (j = 0; j < 8; j++)
    {
      XORResult = CRC & 1;
      CRC >>= 1;
      if (XORResult)
        CRC ^= xorCRC;
    }
  }
}


void parse_task(void *pvParameters)
{

	uint8 op;
	uint8 object;
	uint8 *tempBuf;
	uint8 i = 0;
	
	for(;;)
	{

		if(pkgFlag)
		{
			printf("parse start\n");
			i = 0;
			op = dataBuf[0];
			object = dataBuf[1];
			printf("%x\n",op);
			printf("%x\n",object);
			
			if(op == 0x01) // read
			{
				switch(object)
				{
					case CHIP_ID:
					
						os_printf("get chip id\n");

						tempBuf = (uint8 *)os_malloc(9);
						
						tempBuf[i++] = op;
						tempBuf[i++] = CHIP_ID;
						tempBuf[i++] = 4;
						uint32 a = system_get_chip_id();;
						uint8 *p = (uint8 *)&a;
						
						tempBuf[i++] = p[3];
						tempBuf[i++] = p[2];
						tempBuf[i++] = p[1];
						tempBuf[i++] = p[0];

						uint16 crc = crc16(tempBuf,7);
						p = (uint8 *)&crc;
						
						tempBuf[i++] = p[1];
						tempBuf[i++] = p[0];
						
						sendAck(tempBuf, 13);
						
					break;
						
					case SDK_VERSION:
					
						
						
					break;
						
					case AP_MAC:
						
					break;
						
					case STA_MAC:
						
					break;
						
					case VENDCODE:
						
					break;
						
					case DEVICE:
						
					break;
						
					case HARDWAREVER:
						
					break;
						
					case ID:
						
					break;
						
					case VERSION:
						
					break;
				}
			}
			else if(op == 0x01) // write
			{
				
			}
			else if(op == 0x02) // ack
			{
				
			}
		}

		pkgFlag = FALSE;
		
		vTaskDelay(1000 / portTICK_RATE_MS);
	
	}
	
	// delete the task
	vTaskDelete(NULL);
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	printf("THIS IS A UART LOOPBACK DEMO...\n");
	uart_init_new();    //UART0 Initialize

	xTaskCreate(parse_task, "parse_task", 512, NULL, 3, NULL);
	
}

