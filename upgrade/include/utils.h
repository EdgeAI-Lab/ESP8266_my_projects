#ifndef __UTILS_H__
#define __UTILS_H__
#include "espressif/c_types.h"

typedef struct{
	
	uint8 hour;
	uint8 min;
	uint8 sec;
	
}itime_t;


extern itime_t *time;

uint8 getUrl(char *pusrdata,unsigned short length);
itime_t *getTime(char *pusrdata);
void strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl);
void getDevInfoFromFlash(void);

#endif