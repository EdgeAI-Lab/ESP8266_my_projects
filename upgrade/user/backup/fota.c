/*
 * Tianyuan tec
 *
 *
 *
 */
 
#include "esp_common.h"
#include "lwip/mem.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "upgrade.h"
#include "fota.h"
#include "user_config.h"
#include "espressif/c_types.h"

/*********************global param define start ******************************/
LOCAL os_timer_t upgrade_timer;
LOCAL uint32 totallength = 0;
LOCAL uint32 sumlength = 0;
xTaskHandle *ota_task_handle = NULL;

LOCAL bool socketIsClosed = FALSE;
/*********************global param define end *******************************/

/******************************************************************************
 * FunctionName : upgrade_recycle
 * Description  : recyle upgrade task, if OTA finish switch to run another bin
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void upgrade_recycle(void)
{
	
	//printf("Enter upgrade recycle\n");
	
	totallength = 0;
    sumlength = 0;
    
	initMemary();
	
    system_upgrade_deinit();
	
	//printf("system upgrade deinit\n");

    if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
		
		printf("reboot start.\n");
		
        system_upgrade_reboot(); // if need
		
		//printf("rebooting...\n");
    }
	
	
	 printf("resume get time task\n");
	 xSemaphoreGive( xSemaphore );
	 //vTaskResume(time_task_handle);
	 
	 vTaskDelay(2000 / portTICK_RATE_MS);
	
	 printf("Delete upgrade task!\n");
	 vTaskDelete(ota_task_handle);

}

/******************************************************************************
 * FunctionName : upgrade_download
 * Description  : parse http response ,and download remote data and write in flash
 * Parameters   : int sta_socket : ota client socket fd
 *                char *pusrdata : remote data
 *                length         : data length
 * Returns      : none
*******************************************************************************/
void upgrade_download(int sta_socket,char *pusrdata, unsigned short length)
{
    char *ptr = NULL;
    char *ptmp2 = NULL;
    char lengthbuffer[32];
    if (totallength == 0 && (ptr = (char *)strstr(pusrdata, "\r\n\r\n")) != NULL &&
            (ptr = (char *)strstr(pusrdata, "Content-Length")) != NULL) {
        ptr = (char *)strstr(pusrdata, "\r\n\r\n");
        length -= ptr - pusrdata;
        length -= 4;  // 本次接收到的bin文件的长度（因为bin文件可能一次接收不完）
        printf("upgrade bin file download start.\n");
		
        ptr = (char *)strstr(pusrdata, "Content-Length: ");
        if (ptr != NULL) {
            ptr += 16;
            ptmp2 = (char *)strstr(ptr, "\r\n");

            if (ptmp2 != NULL) {
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                memcpy(lengthbuffer, ptr, ptmp2 - ptr);
				
				printf("Memory set! \n");
				
                sumlength = atoi(lengthbuffer); // 得到bin文件的实际大小（数据来自服务器）
				printf("sumlength = %d\n",sumlength);
                if(sumlength > 0) {
                	if (false == system_upgrade(pusrdata, sumlength)) { // 擦除Flash（大小为bin文件的大小）
                		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
                        goto ota_recycle;
                	}
					
					printf("flash easer over\n");
                	ptr = (char *)strstr(pusrdata, "\r\n\r\n"); 
					
					// 将指针移动到bin文件的起始位置，并将本次接收到的bin文件内容写进flash
                	if (false == system_upgrade(ptr + 4, length)){ 
                		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
                        goto ota_recycle;
                	}
					
					// 统计接收到的（写入flash的）数据的长度，后面要与bin文件的实际长度作比较
                	totallength += length; 
                	
                	return; // 本次写完flash后返回
                }
            } else {
                printf("sumlength failed\n");
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
                goto ota_recycle;
            }
        } else {
            printf("Content-Length: failed\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
            goto ota_recycle;
        } 
    } else {
		
        totallength += length;
        printf("totallen = %d\n",totallength);
        if (false == system_upgrade(pusrdata, length)){
        	system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
            goto ota_recycle;
        }
		
		// 接收到的数据长度和bin文件实际长度相等，则表明数据接收完成
        if (totallength == sumlength) {
	        printf("upgrade file download finished.\n");
			
			//检验数据完整性
	        if(upgrade_crc_check(system_get_fw_start_sec(),sumlength) != true) {
				printf("upgrade crc check failed !\n");
		        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
		        goto ota_recycle;
		    }

			//数据接收完成，关闭socket
			close(sta_socket);
			printf("fota Socket closed!\n");
			socketIsClosed = TRUE;
			system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
			
			upgrade_recycle();
			
			
        } else {
            return;
        }
    }
	
ota_recycle :
		printf("go to ota recycle\n");
        close(sta_socket);
		socketIsClosed = TRUE;
        upgrade_recycle();
}


/*
void freeMemory(void)
{
	
	 free(pbuf);
     pbuf=NULL;

}*/

/******************************************************************************
 * FunctionName : fota_begin
 * Description  : ota_task function
 * Parameters   : task param
 * Returns      : none
*******************************************************************************/
void fota_begin(void *pvParameters)
//void fota_begin(void)
{


    int recbytes;
    int sin_size;
    int sta_socket;
    struct hostent *host;
    //uint8 user_bin[9] = {0};
    struct sockaddr_in remote_ip;
    printf("fota begin!\r\n");
	

    while (1) {
	    sta_socket = socket(PF_INET,SOCK_STREAM,0);
	    if (-1 == sta_socket)
	    {
	        close(sta_socket);
	        printf("fota socket fail !\r\n");
	        continue;
	    }
	    printf("fota socket ok!\r\n");
	    bzero(&remote_ip,sizeof(struct sockaddr_in));
		
		host = gethostbyname(domain);
		
	    remote_ip.sin_family=AF_INET;
	    remote_ip.sin_addr.s_addr= inet_addr(SERVER_IP);
		//remote_ip.sin_addr = *((struct in_addr *)host->h_addr);
	    remote_ip.sin_port=htons(SERVER_PORT);

	    if(0 != connect(sta_socket,(struct sockaddr *)(&remote_ip),sizeof(struct sockaddr)))
	    {
	        close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
	        printf("fota socket connect fail!\r\n");
	        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
	        //upgrade_recycle();
			continue;
	    }
	    printf("fota socket connect ok!\r\n");
		
		char *fota_pbuf = (char *)malloc(512);
		//bzero(pbuf,512);
	    //sprintf(fota_pbuf, "GET /%s HTTP/1.0\r\nHost: %s:%d\r\n"pheadbuffer"",
	                   //srcPath, domain, 80);
					   
		sprintf(fota_pbuf, "GET /%s HTTP/1.0\r\nHost: %s:%d\r\n"pheadbuffer"",
	                   downloadUrl, SERVER_IP, 80);
					   
		//输出GET请求
	    //printf(pbuf);
		
        if(write(sta_socket,fota_pbuf,strlen(fota_pbuf)+1) < 0) {
	            close(sta_socket);
				vTaskDelay(1000 / portTICK_RATE_MS);
	            printf("fota get request send fail\n");
	            free(fota_pbuf);
				fota_pbuf=NULL;
	            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
	            //upgrade_recycle();
				continue;
				
	    }
		
        printf("fota get request send success\n");
		
		free(srcPath);
		srcPath = NULL;
		
		free(domain);
		domain = NULL;
		
		//
		free(fota_pbuf);
	    fota_pbuf=NULL;
		
		
		char *fota_recv_buf = (char *)malloc(1460);
		//bzero(recv_buf,1460);
        while((recbytes = read(sta_socket ,fota_recv_buf,1460)) > 0){  

			printf("loading bin file! \n");
            upgrade_download(sta_socket,fota_recv_buf,recbytes);
        }
		
		//
		free(fota_recv_buf);
		fota_recv_buf=NULL;
		
		if(socketIsClosed == FALSE)
		{
			close(sta_socket);
		}
		
		printf("upgrade fail\n");
		

		upgrade_recycle();
		//break;
		
    }
}

