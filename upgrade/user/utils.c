/*
 * File : utils.c
 *
 *
 */
 
#include "utils.h"
#include "esp_common.h"
#include "lwip/sockets.h"
#include "espressif/upgrade.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "json/cJSON.h"
#include "lwip/netdb.h"
#include "espconn.h"
#include "user_config.h"
#include "espressif/c_types.h"

char *venderCode = NULL;
char *productCode = NULL;
char *hardware = NULL;


/******************************************************************************
 * FunctionName : getDevInfoFromFlash
 * Description  : get device info from flash
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void getDevInfoFromFlash(void)
{
	
	  uint8 len = 0;
	  uint32 value[10]={0};
	  char *s = (char *)&value;
	  spi_flash_read(0x1FC * SPI_FLASH_SEC_SIZE, (uint32 *)&value, 40);
		
       char *p = NULL;
       p=strtok(s,"*");
       if(p)
       {
		  len = strlen(p)+1;
		  venderCode = (char *)os_malloc(len);
		  memset(venderCode,0,len);
		  memcpy(venderCode,p,len);
          printf("%s\n",venderCode);
       }

       p=strtok(NULL,"*");
       if(p)
       {

		  len = strlen(p)+1;
		  productCode = (char *)os_malloc(len);
		  memset(productCode,0,len);
		  memcpy(productCode,p,len);
          printf("%s\n",productCode);
       }

       p=strtok(NULL,"*");
       if(p)
       {
		  
		  len = strlen(p)+1;
		  hardware = (char *)os_malloc(len);
		  memset(hardware,0,len);
		  memcpy(hardware,p,len);
          printf("%s\n",hardware);
       }
	   
	   if( (venderCode == NULL) || (productCode == NULL) || (hardware == NULL) )
	   {
		   printf("use default device info\n");
			venderCode = "HUADI";
			productCode = "HDZT00A1";
			hardware = "V-1-2";
	   }	
}

/******************************************************************************
 * FunctionName : getTime
 * Description  : get the real time from server 
 * Parameters   : char *pusrdata
 * Returns      : none
*******************************************************************************/
itime_t *getTime(char *pusrdata)
{

	char *ptr = NULL;
	
	memset(time,0,sizeof(itime_t));
	char timeBuf[2];
		
	if( (ptr = (char *)strstr(pusrdata, "Date")) != NULL )
	{
		
		// hour
		ptr += 23;
		memcpy(timeBuf,ptr,2);
		time->hour = (uint8)atoi(timeBuf)+8;
		//os_printf("%s\n",timeBuf);
		//os_printf("%d\n",time->hour);
		
		//min
		ptr += 3;
		memcpy(timeBuf,ptr,2);
		time->min = (uint8)atoi(timeBuf);
		//os_printf("%s\n",timeBuf);
		//os_printf("%d\n",time->min);
		
		//sec
		ptr += 3;
		memcpy(timeBuf,ptr,2);
		time->sec = (uint8)atoi(timeBuf);
		//os_printf("%s\n",timeBuf);
		//os_printf("%d\n",time->sec);
		
		
		return time;
	}
	else
	{
		return NULL;
	}

}


uint8 getUrl(char *pusrdata,unsigned short length)
{

	char *ptr = NULL;
	char *ptmp2 = NULL;
	char lengthbuffer[32];

    uint32 sumlength = 0;
	
	if ((ptr = (char *)strstr(pusrdata, "\r\n\r\n")) != NULL &&
		(ptr = (char *)strstr(pusrdata, "Content-Length")) != NULL) {
			
			
		ptr = (char *)strstr(pusrdata, "\r\n\r\n");
        length -= ptr - pusrdata;
        length -= 4;  

        ptr = (char *)strstr(pusrdata, "Content-Length: ");
        if (ptr != NULL) {
            ptr += 16;
            ptmp2 = (char *)strstr(ptr, "\r\n");

            if (ptmp2 != NULL) {
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                memcpy(lengthbuffer, ptr, ptmp2 - ptr);
				
                sumlength = atoi(lengthbuffer); // 得到json文件的实际大小（数据来自服务器）
				
				//os_printf("%d\n",sumlength);
				
				
				if(sumlength == 1)
				{
					return 0;
				}
				
                if(sumlength == length) {
                	
					ptr = (char *)strstr(pusrdata, "\r\n\r\n")+4;
					
					if(ptr != NULL)
					{
						if( (strstr(ptr,"http")!=NULL) && (strstr(ptr,".bin")!=NULL) )
						{
							//os_printf("url: %s\n",url);
							char *domainEnd =  strstr(ptr+7,"/");
							
							
							//memset(pDstOut,0,50);
							//strrpl(pDstOut, domainEnd, "\r\n", "");
							
							// srcPath global var
							
							memset(srcPath,0,50);
							
							//strcpy(srcPath,pDstOut);
							strcpy(srcPath,domainEnd);
							os_printf("srcPath : %s",srcPath);
							
							//os_free(pDstOut);
							
							uint8 domainLen = domainEnd - (ptr+7);

							// domain global var
							
							memset(domain,0,50);
							
							memcpy(domain,ptr+7,domainLen);
							
							os_printf("domain : %s",domain);
							
							return 1;

						
						}	
					}
                }
            } 	
        } 
    }

	return 0;
}



/******************************************************************************
 * FunctionName : strrpl
 * Description  : replace string 
 * Parameters   : char* pDstOut       : the memory for output
 *                char* pSrcIn        : the origin string
 *                const char* pSrcRpl : 将被替换的字符串
 *                const char* pDstRpl : 替换字符串
 * Returns      : none
*******************************************************************************/
void strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl)
{
	char* pi = pSrcIn;
	char* po = pDstOut;

	int nSrcRplLen = strlen(pSrcRpl);
	int nDstRplLen = strlen(pDstRpl);

	char *p = NULL;
	int nLen = 0;

	do
	{
		// 找到下一个替换点
		p = strstr(pi, pSrcRpl);

		if(p != NULL)
		{
		// 拷贝上一个替换点和下一个替换点中间的字符串
		nLen = p - pi;
		memcpy(po, pi, nLen);

		// 拷贝需要替换的字符串
		memcpy(po + nLen, pDstRpl, nDstRplLen);
		}
		else
		{
		strcpy(po, pi);

		// 如果没有需要拷贝的字符串,说明循环应该结束
		break;
		}

		pi = p + nSrcRplLen;
		po = po + nLen + nDstRplLen;

	} while (p != NULL);
}
