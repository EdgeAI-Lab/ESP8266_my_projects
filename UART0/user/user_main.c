/*
 * Tianyuan tec.
 *
 */

#include "esp_common.h"
#include "uart.h"
#include "user_config.h"
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


/*****************************************************************************

uart_init_new(); Initalizes UART0 & UART1 of ESP8266 (You can change the Baud rates & Structure of the UART frame here)
***UART1 only has a single Tx Pin. There is no Receiver at UART1

uart0_tx_buffer(str pointer, length); Sends the Data if the UART Ports are Initialized 
Changes made in the Library : Changed the Interrupt Handler Function : uart0_rx_intr_handler for Receiving Data in data buffer 

*************************************************************************************/

/************golbal variable********************/
bool pkgFlag = FALSE;
uint8 uart_rx_buf[40];

/*
 * 数据帧处理
 *
 *
 */
void data_handle(uint8 length)
{
	
	switch(uart_rx_buf[ENDPOINT_ID])
	{

		
		case 0x01: // ESP8266
			
			// read info
			if( uart_rx_buf[CMD] == READ )
			{
				printf("read endpoint 8266\n");
				Get_ESP8266_Cluster_Info((uart_rx_buf[CLUSTER_ID]<<8)+uart_rx_buf[CLUSTER_ID+1]);
			}
			// write info
			else if( uart_rx_buf[CMD] == WRITE )
			{
				printf("write endpoint 8266\n");
			}
		
		break;
		
		
		case 0x02: // flash
		
			// read info
			if( uart_rx_buf[CMD] == READ )
			{
				printf("read endpoint flash\n");
				getFlashClusterInfo((uart_rx_buf[CLUSTER_ID]<<8)+uart_rx_buf[CLUSTER_ID+1]);
			}
			// write info
			else if( uart_rx_buf[CMD] == WRITE )
			{
				printf("write endpoint flash\n");
				setFlashClusterInfo((uart_rx_buf[CLUSTER_ID]<<8)+uart_rx_buf[CLUSTER_ID+1]);
			}
		
		break;
		
		default:
			printf("no port\n");
		break;
		
	}
	
}


/*****************************************************************************
函数名称 : get_check_sum
功能描述 : 计算校验和
输入参数 : pack:数据源指针
           pack_len:计算校验和长度
返回参数 : 校验和
*****************************************************************************/
unsigned char get_check_sum(unsigned char *pack, unsigned short pack_len)
{
  unsigned short i;
  unsigned char check_sum = 0;
  
  for(i = 0; i < pack_len; i ++)
  {
    check_sum += *pack ++;
  }
  
  return check_sum;
}


xSemaphoreHandle timerSemaphore = NULL;
uint32  frameStartTime = 0;

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	wifi_set_opmode(STATION_MODE);
	{
		struct station_config config;
		bzero(&config, sizeof(struct station_config));
		sprintf(config.ssid, DEMO_WIFI_SSID);
		sprintf(config.password, DEMO_WIFI_PASSWORD);
		wifi_station_set_config(&config);
		//wifi_station_connect();
	}
	
	//UART0 初始化
	uart_init_new();  
	
	// 创建信号量
	vSemaphoreCreateBinary( timerSemaphore );

	// 
	xSemaphoreTake( timerSemaphore, portMAX_DELAY );
	
	// 创建串口数据帧接收超时检测任务(源码在uart.c中)
	xTaskCreate(timer_task, "timer_task", 512, NULL, 3, NULL);
	
}

