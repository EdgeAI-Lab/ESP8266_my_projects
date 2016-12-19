/*
 * Tianyuan tec.
 *
 */

#ifndef __FLASH_H__
#define __FLASH_H__

void getFlashAttrValue(uint16 attrID);
void getFlashClusterInfo(uint16 clusterID);
bool compareLisence(uint8 *buf);
void setLisence(void);
void setFlashAttrValue(uint16 attrID);
void setFlashClusterInfo(uint16 clusterID);
unsigned long byte_to_int(const unsigned char value[4]);



#endif