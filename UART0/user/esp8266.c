/*
 * Tianyuan tec.
 *
 */

#include "user_config.h"
#include "esp_common.h"
#include "esp8266.h"


/*****************************************************************************
函数名称 : int_to_byte
功能描述 : 将int类型拆分四个字节
输入参数 : number:4字节原数据;value:处理完成后4字节数据
返回参数 :无
****************************************************************************/
void int_to_byte(unsigned long number,unsigned char value[4])
{
  value[0] = number >> 24;
  value[1] = number >> 16;
  value[2] = number >> 8;
  value[3] = number & 0xff;
}


/*
 * 获取ESP8266 WiFi属性值
 *
 */
void Get_ESP8266_WIFI_AttrValue(uint16 attrID)
{
	uint8 status[1];
	uint8 len;
	
	struct station_config config;
	
	printf("%x\n",attrID);
	
	switch(attrID)
	{
		case WIFI_SSID:
		
			printf("ssid");
			wifi_station_get_config(&config);
			
			// 获取ssid的长度
			for(len=0;config.ssid[len] != '\0';len++);
			
			// 应答
			sendAck(config.ssid,len);
			
			
		break;
		
		case WIFI_PASSWORD:
		
			printf("password");
			wifi_station_get_config(&config);
			
			// 获取ssid的长度
			for(len=0;config.password[len] != '\0';len++);
			
			// 应答
			sendAck(config.password,len);
			
			
		break;
		
		case WIFI_CONNECT_STA:
		
			status[0] = wifi_station_get_connect_status();
			sendAck(status,1);
		
		break;
	}
}

/*
 * 设置ESP8266 WiFi属性值
 *
 */
void Set_ESP8266_WIFI_AttrValue(uint16 attrID)
{
	switch(attrID)
	{
		case WIFI_SSID:
		
			printf("read only\n");
			sendAckNoData();
		
		break;
		
		case WIFI_PASSWORD:
		
			printf("read only\n");
			sendAckNoData();
		
		break;
		
		case WIFI_CONNECT_STA:
		
			printf("read only\n");
			sendAckNoData();
		
		break;
	}
}



/*
 * 获取ESP8266的基本属性值
 *
 */
void Get_ESP8266_Info_AttrValue(uint16 attrID)
{
	uint8 param[4];
	uint8 len;
	switch(attrID)
	{
		case CHIP_ID:
			
			printf("attrID\n");
			
			uint32 chipID = system_get_chip_id();
			
			// 将chipID(uint32)，转换为字节数组
			int_to_byte(chipID,param);

			sendAck(param,4);

		break;
		
		case SDK_VER:
			
			printf("attrID\n");
			
			const uint8 *ver = system_get_sdk_version();
			
			// 得到版本的长度
			for(len=0;ver[len] != '\0';len++);

			sendAck(ver,len);

		break;
		
	}
}


/*
 * 获取ESP8266 Cluster
 *
 */
void Get_ESP8266_Cluster_Info(uint16 clusterID)
{
	switch(clusterID)
	{
		
		case ESP8266_INFO:
			
			printf("clusterID\n");
			Get_ESP8266_Info_AttrValue( (uart_rx_buf[ATTR_ID]<<8) + uart_rx_buf[ATTR_ID+1]);
		
		break;
		
		case ESP8266_WIFI:
			
			printf("WiFi clusterID\n");
			Get_ESP8266_WIFI_AttrValue( (uart_rx_buf[ATTR_ID]<<8) + uart_rx_buf[ATTR_ID+1]);
		
		break;
		
		
	}
}


/*
 *  设置ESP8266基本属性值
 *
 */
void Set_ESP8266_INFO_AttrValue(uint16 attrID)
{
	uint8 param[4];
	
	uint32 chipID;
	
	switch(attrID)
	{
		case CHIP_ID:
			
			printf("read only\n");

			sendAckNoData();

		break;
		
		
		case SDK_VER:
			
			printf("read only\n");

			sendAckNoData();
			
		break;
	}
}


/*
 * 设置ESP8266 Cluster
 *
 */
void Set_ESP8266_Cluster_Info(uint16 clusterID)
{
	switch(clusterID)
	{	
		// 基本信息
		case ESP8266_INFO:
			
			printf("ESP8266_INFO clusterID\n");
			Set_ESP8266_INFO_AttrValue( (uart_rx_buf[ATTR_ID]<<8) + uart_rx_buf[ATTR_ID+1]);
		
		break;
		
		// wifi信息
		case ESP8266_WIFI:
			
			printf("ESP8266_WIFI clusterID\n");
			Set_ESP8266_WIFI_AttrValue( (uart_rx_buf[ATTR_ID]<<8) + uart_rx_buf[ATTR_ID+1]);
		
		break;
			
	}
}

