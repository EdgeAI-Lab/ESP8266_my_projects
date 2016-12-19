/*
 * Tianyuan tec
 *
 *
 *
 */

#include "esp_common.h"
#include "lwip/sockets.h"
#include "espressif/upgrade.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "json/cJSON.h"
#include "lwip/netdb.h"
#include "espconn.h"
#include "utils.h"
#include "get_json.h"
#include "user_config.h"
#include "espressif/c_types.h"
#include "fota.h"



LOCAL uint32 totallength = 0;
LOCAL uint32 sumlength = 0;

char *downloadUrl = NULL;
char *domain = NULL;
char *srcPath = NULL;

LOCAL bool closeSocket = FALSE;

bool UPGRADE_FAIL = TRUE;

/******************************************************************************
 * FunctionName : jsonDatas_load
 * Description  : download the datas of json file and write into memory, then
 *                change the string into a json string.
 * Parameters   : int sta_socket : the socket
 *                char *pusrdata : remote data
 *                length         : data length
 * Returns      : none
*******************************************************************************/
void jsonDatas_load(int sta_socket,char *pusrdata, unsigned short length)
{
		
	char *ptr = NULL;
    char *ptmp2 = NULL;
    char lengthbuffer[32];

	
	if (totallength == 0 && (ptr = (char *)strstr(pusrdata, "\r\n\r\n")) != NULL &&
		(ptr = (char *)strstr(pusrdata, "Content-Length")) != NULL) {
			
        ptr = (char *)strstr(pusrdata, "\r\n\r\n");
        length -= ptr - pusrdata;
        length -= 4;  // 本次接收到的json文件的长度（因为bin文件可能一次接收不完）
        printf("upgrade json file download start.\n");
		
		//printf("lengthMinusHead: %d\n",length);
        
		
		
        ptr = (char *)strstr(pusrdata, "Content-Length: ");
        if (ptr != NULL) {
            ptr += 16;
            ptmp2 = (char *)strstr(ptr, "\r\n");

            if (ptmp2 != NULL) {
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                memcpy(lengthbuffer, ptr, ptmp2 - ptr);
				
                sumlength = atoi(lengthbuffer); // 得到json文件的实际大小（数据来自服务器）
                if(sumlength > 0) {
                	
                	ptr = (char *)strstr(pusrdata, "\r\n\r\n"); 
					
					// 将指针移动到json文件的起始位置，并将本次接收到的json文件内容写打印出来
                	//printf("%s\n",ptr+4);
					memcpy(jsonDatas,ptr+4,length);
					// 移动指针
				    jsonDatas += length;

					// 统计接收到的（写入flash的）数据的长度，后面要与bin文件的实际长度作比较
                	totallength += length; 
                	printf("sumlength = %d\n",sumlength);
                	return; // 本次写完flash后返回
                }
            } else {
                printf("sumlength failed\n");
                
            }
        } else {
            printf("Content-Length: failed\n");
            
        } 
    } else {
        totallength += length;
        printf("totallen = %d\n",totallength);

		memcpy(jsonDatas,pusrdata,length);
		//printf("%s",jsonDatas);
		// 移动指针
		jsonDatas += length;
		
		/* 假如接收到的数据长度和文件的实际长度一样，怎表示文件现在完成 */
        if (totallength == sumlength) {
			
			jsonDatas = jsonStr;
	        printf("json file download finished.\n");
			//printf("%s",jsonDatas);
			close(sta_socket);
			printf("Socket closed!\n");
			closeSocket = TRUE;
			
			totallength = 0;
            sumlength = 0;
	
			bzero(pDstOut,5120);
			/* 将字符串中的 “ \r\n ” 删除 */
			strrpl(pDstOut, jsonStr, "\r\n", "");
			printf("line delete\n");
			
			
			/* 将字符串中的 “ 空格 ” 删除 */
			bzero(pDstOutBlank,4096);
			strrpl(pDstOutBlank, pDstOut, " ", "");
			printf("blank delete\n");
			//printf("%s\n",pDstOutBlank);


			printf("Prase JSON Start!\n");
			//解析JSON字符串
			char *url = parse(pDstOutBlank);
			printf("Prase JSON End!\n");
			
			printf("%s\n",url);

			//char *url = NULL;
			
			if( url != NULL )
			{
				if(strstr(url,"http"))
				{
					printf("url: %s",url);
					char *domainEnd =  strstr(url+7,"/");
					
					// srcPath global var
					srcPath = (char *)zalloc(100);
					
					strcpy(srcPath,domainEnd);

					uint8 domainLen = domainEnd - (url+7);

					// domain global var
					domain = (char *)zalloc(50);

					memcpy(domain,url+7,domainLen);
					
					printf("domain : %s\n",domain);

					//开始FOTA
					freeMemory();
					//fota_begin();
					
					
					
					if (ota_task_handle == NULL) {
						system_upgrade_flag_set(UPGRADE_FLAG_START);
						system_upgrade_init();
						xTaskCreate(fota_begin, "fota_task", 1024, NULL, 2, ota_task_handle);
					}
					
					printf("suspend get time task\n");
					
					xSemaphoreTake( xSemaphore, portMAX_DELAY );
					
					
					
					printf(" get time task resume\n");
				
				}
				else
				{
					downloadUrl = (char *)malloc(75);

					memcpy(downloadUrl,url,75);
				
				
					freeMemory();
					
					//ota_task_handle = NULL;	
					//if (ota_task_handle == NULL) {
						system_upgrade_flag_set(UPGRADE_FLAG_START);
						system_upgrade_init();
						xTaskCreate(fota_begin, "fota_task", 1024, NULL, 2, ota_task_handle);
					//}
					
					printf("suspend get time task\n");
					
					xSemaphoreTake( xSemaphore, portMAX_DELAY );
				
				
					printf(" get time task resume\n");
				}
				
				
				
			}
			else
			{
				printf("Don't need upgrade!\n");
				return;
			}

			

        } else {
			
            return;
        }
		
    }	
	
}


/******************************************************************************
 * FunctionName : getJsonFile_begin
 * Description  : send a GET request to server for get the json file.
 * Parameters   : pvParameters
 * Returns      : none
*******************************************************************************/
//void getJsonFile_begin(void *pvParameters)
void getJsonFile_begin(void)
{
	
	int sta_socket;
	int recbytes;
	struct sockaddr_in remote_ip;
	
	
	
	while(1)
	{
		sta_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (-1 == sta_socket) {
			
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			printf("json task get socket fail!\n");
			continue;

		}
		printf("json task get socket ok!\n");
		
		bzero(&remote_ip, sizeof(struct sockaddr_in));
		
		remote_ip.sin_family = AF_INET;
		remote_ip.sin_addr.s_addr = inet_addr(SERVER_IP);
		//remote_ip.sin_addr = *((struct in_addr *)host->h_addr);
		remote_ip.sin_port = htons(SERVER_PORT);
		
		if (0 != connect(sta_socket, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr))) {
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			printf("get json file socket connect fail!\n");
			continue;
		}
		printf("get json file socket connect ok!\n");

		//char *pbuf = (char *)zalloc(512);
		bzero(pbuf,512);
		sprintf(pbuf, "GET /fhc_update.json HTTP/1.1\r\nHost: %s:%d\r\n"pheadbuffer"",
				SERVER_IP, 80);
		
		if(write(sta_socket,pbuf,strlen(pbuf)+1) < 0) {
					close(sta_socket);
					printf("get json file request send fail\n");
					//free(pbuf);
					continue;
					
		}
		printf("get json file request send success\n");
		//free(pbuf);
		//pbuf=NULL;
		
		
		
		bzero(jsonDatas,5120);
		//printf("data head addr is: %x\n",jsonStr);
		
		//char *recv_buf = (char *)malloc(1460);
		bzero(recv_buf,1460);
		while((recbytes = read(sta_socket ,recv_buf,1460)) > 0){  
				
				printf("loading json file! \n");
				// 将接收到的JSON数据
				//1）写进内存；2）转换成没有\r\n和空格的JSON字符串；3）解析JSON数据 -- 函数parse() --				
				jsonDatas_load(sta_socket,recv_buf,recbytes);
					
				
			}
        
		if(closeSocket == FALSE)
		{
			closeSocket = TRUE;
			close(sta_socket);
		}
		
		/*if(jsonStr != NULL)
		{
			free(jsonStr);
			jsonStr = NULL;
		}*/
		
		//free(recv_buf);
		//jsonDatas=NULL;
		
		
		totallength = 0;
        sumlength = 0;
		
		// 无需升级时，跳出循环
		break;
		
		//printf("delete the get Json File task\n");
		//vTaskDelete(NULL);
		
	}
	
	
	//xTaskCreate(getTime_task, "getTime_task", 1024, NULL, 1, NULL);
	
	
	
	
	
}