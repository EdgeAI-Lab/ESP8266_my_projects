/*
 * Tianyuan tec.
 *
 */
 
#include "user_config.h"
#include "esp_common.h"
#include "flash.h"
#include "upgrade.h"
#include "tea.h"

/*
 * 将一个十六进制数转换为ASCII字符
 * 例如：0xAB12CD45  --->  'A' 'B' '1' '2' 'C' 'D' '4' '5'
 *
 */
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


/*
 * 与上位机发送过来的Lisence相比较
 *
 */
bool compareLisence(uint8 *buf)
{
	uint8 i = 0;
	for(i=0;i<8;i++)
	{
		if(buf[i] != uart_rx_buf[DATA+i])
		{
			return FALSE;
		}
		else
		{
			printf("lisen right\n");
			return TRUE;
		}
	}
}


/*
 * 根据设备信息生成Lisence
 *
 */
void generateLisence(unsigned char *buf)
{
	uint32 crc = crc32("HUADI_GZ6200B1FB3E48588FCB35C3BA3ACA636858280CA175B9",strlen("HUADI_GZ6200B1FB3E48588FCB35C3BA3ACA636858280CA175B9"));
	
	//u8 buf[8];
	
		
	hexToASC(crc,buf);

	encrypt(buf,8,publickey);

}

/*
 * 小端模式
 *
 */
unsigned long byte_to_int(const unsigned char value[4])
{
  uint32 nubmer = 0;

  nubmer = (unsigned long)value[3];
  nubmer <<= 8;
  nubmer |= (unsigned long)value[2];
  nubmer <<= 8;
  nubmer |= (unsigned long)value[1];
  nubmer <<= 8;
  nubmer |= (unsigned long)value[0];
  
  return nubmer;
}


/*
 * 获取flash属性值
 *
 */
void getFlashAttrValue(uint16 attrID)
{
	uint8 *lisenceBuf = NULL;
	uint32 lisence[2] = {0};

	switch(attrID)
	{
		case FLASH_LISENCE:
			
			printf("attrID\n");
			spi_flash_read(0x1FD * 4096, (uint32 *)&lisence, 8);
			
			lisenceBuf = (uint8* )&lisence;
			
			sendAck(lisenceBuf,8);

		break;
	}
}


/*
 * 获取flash cluster
 *
 */
void getFlashClusterInfo(uint16 clusterID)
{
	switch(clusterID)
	{
		
		case FLASH_INFO:
			
			printf("clusterID\n");
			getFlashAttrValue( (uart_rx_buf[ATTR_ID]<<8) + uart_rx_buf[ATTR_ID+1]);
		
		break;
		
		
	}
}


/*
 * ESP8266自己首先生成Lisence，然后与上位机发送过来的Lisence相比较 
 * a)若两个Lisence相等，则重启系统运行应用App
 * b)若两个Lisence不相等，则发送ACK到上位机
 */
void setLisence(void)
{
	uint8 buf[8];
	uint32 lisence[2];
	
	// ESP8266自己首先生成Lisence
	generateLisence(buf);
	
	// 与上位机发送过来的Lisence相比较 
	if ( compareLisence(buf) == TRUE )
	{
		// 将Lisence写进Flash
		
		lisence[0] = byte_to_int(&uart_rx_buf[DATA]);
		lisence[1] = byte_to_int(&uart_rx_buf[13]);
		
		spi_flash_erase_sector (0x1FD);
		spi_flash_write (0x1FD*4*1024, (uint32 *)&lisence, 8);
		
		// 设置标志位
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
			
		// 重启系统，运行user2.bin
		system_upgrade_reboot();
	}
	else
	{
		// sendAck without data
		sendAckNoData();
	}
	
}


/*
 * 设置Flash属性值
 *
 */
void setFlashAttrValue(uint16 attrID)
{
	uint8 param[4];
	
	uint32 chipID;
	
	switch(attrID)
	{
		case FLASH_LISENCE:
			
			printf("attrID\n");
			setLisence();

		break;
	}
}


/*
 * 设置flash cluster
 *
 */
void setFlashClusterInfo(uint16 clusterID)
{
	switch(clusterID)
	{
		
		case FLASH_INFO:
			
			printf("clusterID\n");
			setFlashAttrValue( (uart_rx_buf[ATTR_ID]<<8) + uart_rx_buf[ATTR_ID+1]);
		
		break;
		
		
	}
}