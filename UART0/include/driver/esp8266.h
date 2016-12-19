/*
 * Tianyuan tec.
 *
 */

#ifndef __ESP8266_H__
#define __ESP8266_H__

void Set_ESP8266_Cluster_Info(uint16 clusterID);
void Set_ESP8266_INFO_AttrValue(uint16 attrID);
void Set_ESP8266_WIFI_AttrValue(uint16 attrID);
void Get_ESP8266_Cluster_Info(uint16 clusterID);
void Get_ESP8266_Info_AttrValue(uint16 attrID);
void Get_ESP8266_WIFI_AttrValue(uint16 attrID);



#endif