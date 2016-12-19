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
#include "spi_flash.h"

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


#define DEMO_WIFI_SSID "rhxtune"
#define DEMO_WIFI_PASSWORD "tianyuan"
#define SERVER_IP "10.2.10.41"
#define SERVER_PORT 80


/******************************************************************************
 * FunctionName : test_task
 * Description  : test task.
 * Parameters   : pvParameters
 * Returns      : none
*******************************************************************************/
void test_task(void *pvParameters)
{
	/*  user code */
    
	vTaskDelete(NULL);
	printf("delete the websocket_task\n");

}


/******************************************************************************
 * FunctionName : fota_begin
 * Description  : ota_task function
 * Parameters   : task param
 * Returns      : none
*******************************************************************************/
void wifi_handle_event_cb(System_Event_t *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
            os_printf("sta got ip , creat json prase task\n");
         	xTaskCreate(test_task, "test_task", 4096, NULL, 1, NULL);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            printf("sta disconnect from AP\n");
        default:
            break;
    }
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
   /* wifi_set_opmode(STATION_MODE);
	{
        struct station_config config;
        bzero(&config, sizeof(struct station_config));
        sprintf(config.ssid, DEMO_WIFI_SSID);
        sprintf(config.password, DEMO_WIFI_PASSWORD);
        wifi_station_set_config(&config);
        wifi_station_connect();
	}
	wifi_set_event_handler_cb(wifi_handle_event_cb);*/
	
	spi_flash_erase_sector(0x1FD);
	//spi_flash_erase_sector(0x1FE);
	//spi_flash_erase_sector(0x1FF);
	
	
	//uint32 value = 0x12345678;
	
	//spi_flash_write (0x1FD*4*1024, (uint32 *)&value, 4);
	
	

	//uint8 info[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
	
	//system_param_save_with_protect(0x1FD,&info,8);
	
	
	
	
	//uint32 value[2]={0};
	char *addr = (char *)&value;
	//system_param_load(0x1FD,0,(uint32 *)&value,8);
	
	//printf("0x1FD sec:%02x%02x%02x%02x%02x%02x%02x%02x\r\n", addr[0], addr[1], addr[2], addr[3],addr[4], addr[5], addr[6], addr[7]);
	
	spi_flash_read(0x1FD * 4096, (uint32 *)&value, 4);
	
	printf("0x1FD sec:%02x%02x%02x%02x\r\n", addr[0], addr[1], addr[2], addr[3]);
	
	/*spi_flash_read(0x1FE * 4096, (uint32 *)&value, 8);
	
	printf("0x1FE sec:%02x%02x%02x%02x%02x%02x%02x%02x\r\n", addr[0], addr[1], addr[2], addr[3],addr[4], addr[5], addr[6], addr[7]);
	
	spi_flash_read(0x1FF * 4096, (uint32 *)&value, 16);
	
	printf("0x1FF sec:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n", addr[0], addr[1], addr[2], addr[3],addr[4], addr[5], addr[6], addr[7],addr[8], addr[9], addr[10], addr[11],addr[12], addr[13], addr[14], addr[15]);

	*/
	
	
	/*uint32 rtc_t = system_get_rtc_time();
	uint32 cal = system_rtc_clock_cali_proc();
	os_printf("cal: %d.%d \r\n", ((cal*1000)>>12)/1000, ((cal*1000)>>12)%1000 );*/
	
}

