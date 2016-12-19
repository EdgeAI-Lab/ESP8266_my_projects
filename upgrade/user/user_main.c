/*
 * Tianyuan tec
 *
 *
 *
 */

#include "esp_common.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "espressif/upgrade.h"
#include "freertos/task.h"
#include "lwip/netdb.h"
#include "lwip/apps/sntp_time.h"
#include "espconn.h"
#include "espressif/c_types.h"
#include "user_config.h"
#include "utils.h"
#include "fota.h"


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

/******************************************
 *
 *  var
 *
 */
 
xSemaphoreHandle getimeSemaphore = NULL;
xSemaphoreHandle fotaSemaphore = NULL;

xTaskHandle *time_task_handle = NULL;

char *srcPath = NULL;
char *domain = NULL;

char *pbuf = NULL;
char *recv_buf = NULL;
//char *pDstOut = NULL;

itime_t *time = NULL;

/******************************************************************************
 * FunctionName : getTime_task
 * Description  : getTime task.
 * Parameters   : pvParameters
 * Returns      : none
*******************************************************************************/
void getTime_task(void *pvParameters)
{
	
	int sta_socket;
	int recbytes;
	struct hostent *host;
	struct sockaddr_in *remote_ip;
	
	os_printf("get time task start.\n");
	
	xSemaphoreTake( getimeSemaphore, portMAX_DELAY );
	xSemaphoreTake( fotaSemaphore, portMAX_DELAY );
	
	for(;;)
	{

		sta_socket = socket(PF_INET, SOCK_STREAM, 0);
	
		if (-1 == sta_socket) {
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("time task get socket fail!\n");
			continue;
			
		}

		os_printf("time task get socket ok!\n");
		remote_ip = (struct sockaddr_in *)os_malloc( sizeof(struct sockaddr_in) );
		memset(remote_ip, 0,sizeof(struct sockaddr_in));
		
		if( (host = gethostbyname("license.5itianyuan.com")) == NULL)
		{
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("get time get host ip fail!\n");
			
			continue;
		}
		os_printf("get time got host ip\n");

		remote_ip->sin_family = AF_INET;
		//remote_ip->sin_addr.s_addr = inet_addr(SERVER_IP);
		remote_ip->sin_addr = *((struct in_addr *)host->h_addr);
		remote_ip->sin_port = htons(80);
		
		if (0 != connect(sta_socket, (struct sockaddr *)(remote_ip), sizeof(struct sockaddr))) {
			close(sta_socket);
			
			free(remote_ip);
			remote_ip=NULL;
			
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("get time socket connect fail!\n");
			
			continue;
		
		}
		
		free(remote_ip);
		remote_ip=NULL;
		
		os_printf("get time socket connect ok!\n");
		
		//char *pbuf = (char *)malloc(512);
		memset(pbuf,0,512);			
		sprintf(pbuf, "GET /licenseApi/getVersion?vendcode=%s&productcode=%s&hardware=%s&version=HDZT00A1-20161021-1&id=H1ZT121603300012 HTTP/1.1\r\nHost: %s:%d\r\n"pheadbuffer"",
				venderCode,productCode,hardware,"license.5itianyuan.com", 80);
				
		//sprintf(pbuf, "GET /fhc_update.txt HTTP/1.1\r\nHost: %s:%d\r\n"pheadbuffer"",
				//SERVER_IP, SERVER_PORT);	
		os_printf("%s\n",pbuf);
		
		if(write(sta_socket,pbuf,strlen(pbuf)+1) < 0) {
			
			close(sta_socket);
			os_printf("get time request send fail\n");
			//free(pbuf);
			//pbuf=NULL;
			
			continue;
					
		}
		
		os_printf("get time request send success\n");
		//free(pbuf);
		//pbuf=NULL;

		//char *recv_buf = (char *)malloc(1460);
		memset(recv_buf,0,1460);
		
		// url文件内容，一次即可读完
		recbytes = read(sta_socket ,recv_buf,1460);
		
		os_printf("%s\n",recv_buf);
		
		// 接收数据完毕，关闭socket
		close(sta_socket);
		
		// 获取时间
		itime_t *time = getTime(recv_buf);
        
		if(time != NULL)
		{
			os_printf("%d : %d : %d\n",time->hour,time->min,time->sec);

			if( (time->hour >= UP_HOUR) && (time->hour < DOWN_HOUR) )
			{
				
				//os_free(time);
				//time = NULL;

				 // get url
				 uint8 res = getUrl(recv_buf,recbytes);
				 
				 os_printf("%d\n",res);
				 
				 if(res)
				 {
					system_upgrade_flag_set(UPGRADE_FLAG_START);
					system_upgrade_init();
					xSemaphoreGive( fotaSemaphore ); 
					
					os_printf("suspend get time task\n");
					xSemaphoreTake( getimeSemaphore, portMAX_DELAY );
					os_printf(" get time task resume\n");
					vTaskDelay(10000 / portTICK_RATE_MS);
					
				 }
				 else
				 {
					 os_printf("don't upgrade\n");
					 vTaskDelay(10000 / portTICK_RATE_MS);
				 }
					
				 
				 //free(recv_buf);
		         //recv_buf=NULL;
			}
			else
			{
				os_printf("don't upgrade no in time\n");
				//os_free(time);
				//time = NULL;
				vTaskDelay(10000 / portTICK_RATE_MS);	
			}

		}
		else
		{
			os_printf("get time file\n");
			vTaskDelay(10000 / portTICK_RATE_MS);	
		}
		
	}
	
	os_printf("delete time task\n");
	// delete the task
	vTaskDelete(NULL);

}

/******************************************************************************
 * FunctionName : test_task
 * Description  : test task
 * Parameters   : pvParameters
 * Returns      : none
*******************************************************************************/
void test_task(void *pvParameters)
{
	uint32 heap_size = 0;
	uint32 uxHighWaterMark = 0;
	
	for(;;)
	{
		heap_size = system_get_free_heap_size();
		os_printf("Memory size: %d\n",heap_size);
		
		uxHighWaterMark = uxTaskGetStackHighWaterMark(time_task_handle);
		os_printf("time task stack size: %d\n",uxHighWaterMark);
		
		vTaskDelay(1000 / portTICK_RATE_MS);
		
	}
	
	// delete the task
	vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : wifi_handle_event_cb
 * Description  : wifi handle event
 * Parameters   : System_Event_t *event
 * Returns      : none
*******************************************************************************/
void wifi_handle_event_cb(System_Event_t *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->event_id) {
        case EVENT_STAMODE_GOT_IP:
            //os_printf("sta got ip , creat main task & get time task\n");
         	xTaskCreate(test_task, "test_task", 128, NULL, 1, NULL);
			
			// 初始化内存
			while(!initMem());
			
			vSemaphoreCreateBinary( getimeSemaphore );
	
			vSemaphoreCreateBinary( fotaSemaphore );
			
			xTaskCreate(getTime_task, "getTime_task", 512, NULL, 5, time_task_handle);
			xTaskCreate(fota_begin, "fota_task", 512, NULL, 3, ota_task_handle);
			
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            //os_printf("sta disconnect from AP\n");
			
			system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_recycle();
			
			// delete task
			vTaskDelete(time_task_handle);
			vTaskDelete(ota_task_handle);
			
			wifi_set_opmode(STATION_MODE);
			{
				struct station_config config;
				memset(&config,0, sizeof(struct station_config));
				sprintf(config.ssid, DEMO_WIFI_SSID);
				sprintf(config.password, DEMO_WIFI_PASSWORD);
				wifi_station_set_config(&config);
				wifi_station_connect();
			}
			wifi_set_event_handler_cb(wifi_handle_event_cb);
			
			break;
			
        default:
            break;
    }
}


/************************************************************************************
 *
 *
 *
 *
 *
 ************************************************************************************/
uint8 initMem(void)
{
	// 用于存储get
	pbuf = (char *)os_zalloc(512);
	if(pbuf == NULL)
	{
		return 0;
	}
	
	// 存储接收数据
	recv_buf = (char *)os_zalloc(1460);
	if(recv_buf == NULL)
	{
		return 0;
	}
	
	//pDstOut = (char *)malloc(50); 
	
	srcPath = (char *)os_zalloc(50);
	if(srcPath == NULL)
	{
		return 0;
	}
	
	domain = (char *)os_zalloc(50);
	if(domain == NULL)
	{
		return 0;
	}
	
	time = (itime_t*)os_zalloc(sizeof(itime_t));
	if(time == NULL)
	{
		return 0;
	}
	
	return 1;
}


/************************************************************************************
 *
 *
 *
 *
 *
 ************************************************************************************/
/*void freeMemory(void)
{
	
	os_free(pbuf);
	os_free(recv_buf);
	pbuf=NULL;
	recv_buf=NULL;

}*/

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{

	getDevInfoFromFlash();

	system_print_meminfo();


	wifi_set_opmode(STATION_MODE);
	{
		struct station_config config;
		bzero(&config, sizeof(struct station_config));
		sprintf(config.ssid, DEMO_WIFI_SSID);
		sprintf(config.password, DEMO_WIFI_PASSWORD);
		wifi_station_set_config(&config);
		//wifi_station_connect();
	}
	wifi_set_event_handler_cb(wifi_handle_event_cb);	

}

