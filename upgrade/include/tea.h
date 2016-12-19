/*********************************************************************
*说明:TEA加密解密算法
*TEA(Tiny Encryption Algorithm)是一种简单高效的加密算法，以加密解密速度快，
*实现简单著称。
*算法很简单，TEA算法每一次可以操作64-bit(8-byte)，采用128-bit(16-byte)作为key，
*算法采用迭代的形式，
*推荐的迭代轮数是64轮，最少32轮。
**********************************************************************/

#ifndef _TEA_H_
#define _TEA_H_

/*********************************************************************
*             函数
**********************************************************************/	 
#include "stdlib.h"
#include "esp_common.h"
/*********************************************************************
*             tea加密
*参数:v:要加密的数据,长度为8字节
*     k:加密用的key,长度为16字节
**********************************************************************/

static void tea_encrypt(int32 *v,int32 *k);

/*********************************************************************
*             tea解密
*参数:v:要解密的数据,长度为8字节
*     k:解密用的key,长度为16字节
**********************************************************************/

static void tea_decrypt(int32 *v,int32 *k);

/*********************************************************************
*             加密算法
*参数:src:源数据,所占空间必须为8字节的倍数.加密完成后密文也存放在这
*     size_src:源数据大小,单位字节
*     key:密钥,16字节
*返回:密文的字节数
**********************************************************************/

u16 encrypt(u8 *src,u16 size_src,u8 *key);

/*********************************************************************
*             解密算法
*参数:src:源数据,所占空间必须为8字节的倍数.解密完成后明文也存放在这
*     size_src:源数据大小,单位字节
*     key:密钥,16字节
*返回:明文的字节数,如果失败,返回0
**********************************************************************/

u16 decrypt(u8 *src,u16 size_src,u8 *key);
extern u8 publickey[16];
#endif

