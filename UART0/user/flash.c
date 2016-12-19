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
 * ��һ��ʮ��������ת��ΪASCII�ַ�
 * ���磺0xAB12CD45  --->  'A' 'B' '1' '2' 'C' 'D' '4' '5'
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
 * ����λ�����͹�����Lisence��Ƚ�
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
 * �����豸��Ϣ����Lisence
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
 * С��ģʽ
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
 * ��ȡflash����ֵ
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
 * ��ȡflash cluster
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
 * ESP8266�Լ���������Lisence��Ȼ������λ�����͹�����Lisence��Ƚ� 
 * a)������Lisence��ȣ�������ϵͳ����Ӧ��App
 * b)������Lisence����ȣ�����ACK����λ��
 */
void setLisence(void)
{
	uint8 buf[8];
	uint32 lisence[2];
	
	// ESP8266�Լ���������Lisence
	generateLisence(buf);
	
	// ����λ�����͹�����Lisence��Ƚ� 
	if ( compareLisence(buf) == TRUE )
	{
		// ��Lisenceд��Flash
		
		lisence[0] = byte_to_int(&uart_rx_buf[DATA]);
		lisence[1] = byte_to_int(&uart_rx_buf[13]);
		
		spi_flash_erase_sector (0x1FD);
		spi_flash_write (0x1FD*4*1024, (uint32 *)&lisence, 8);
		
		// ���ñ�־λ
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
			
		// ����ϵͳ������user2.bin
		system_upgrade_reboot();
	}
	else
	{
		// sendAck without data
		sendAckNoData();
	}
	
}


/*
 * ����Flash����ֵ
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
 * ����flash cluster
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