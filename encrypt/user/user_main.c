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
#include "tea.h"

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


uint32 crc32(const unsigned char *buf, uint32 size);


void hexToASC(unsigned int data, unsigned char *dst)
{
	
	unsigned char i = 0;
	unsigned char str[8];

	
	for(i=0;i<8;i++)
	{
		str[i] = data & 0x0F;
		data >>= 4;

	}

	for(i = 0; i<8;i++)
	{
		sprintf(&dst[i],"%X/n",str[7-i]);
	}

}



void encryption_task(void *pvParameters)
{
	uint32 crc = crc32("HUADI_GZ6200B1FB3E48588FCB35C3BA3ACA636858280CA175B9",strlen("HUADI_GZ6200B1FB3E48588FCB35C3BA3ACA636858280CA175B9"));
	
	//printf("%x\n",crc);
	
	
	
		u8 i;

		u8 buf[8];
		
		hexToASC(crc,buf);

		encrypt(buf,8,publickey);

		for(i=0;i<8;i++)
		{
			printf("%x\n", buf[i]);
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
    //printf("SDK version:%s\n", system_get_sdk_version());
	//printf("ESP8266 chip ID:0x%x\n", system_get_chip_id());
	
	
	/*char *msg = "123";
	int i;
	uint8_t result[16];
	
	// benchmark
    for (i = 0; i < 1; i++) {
        md5((uint8_t*)msg, 3, result);
    }
 
    // display result
    for (i = 0; i < 16; i++)
        os_printf("%02x", result[i]);
	
	printf("\n");*/
	
	xTaskCreate(encryption_task, "encryption_task", 512, NULL, 3, NULL);
	
}

