/*
 * CRC.c
 *
 *  Created on: 2015年12月26日
 *      Author: openstack
 */

#include "CRC.h"

unsigned char rfid_checksum(unsigned char buf[])
{
	char	i=0;
	unsigned char	checksum = 0;

	for(i=0;i<buf[1]-1;i++)
	{
		checksum ^= buf[i];
	}
	return ~checksum;
}


/**
 * 查表法
 */
uint CRC32Software(unsigned char * pData, int Length)
{
	uint uCRCValue;
	unsigned char chtemp;
	int i,uSize;

	uSize = Length/4;
	uCRCValue = 0xFFFFFFFF;
	while (uSize --)
	{
		for(i = 3;i >= 0;i--)
		{
			chtemp=*(pData + i);
			uCRCValue = Crc32Table[((uCRCValue>>24)&0xFF) ^ chtemp] ^ (uCRCValue<<8);
		}
		pData += 4;
	}
	return uCRCValue;
}


unsigned short CRC16Software(unsigned char* pDataIn, int iLenIn)
{
	unsigned short wResult = 0;
	unsigned short wTableNo = 0;
	int i = 0;
	for( i = 0; i < iLenIn; i++)
	{
		wTableNo = ((wResult & 0xff) ^ (pDataIn[i] & 0xff));
		wResult = ((wResult >> 8) & 0xff) ^ wCRC16Table[wTableNo];
	}
	return wResult;
}
uint CRC16Modbus ( unsigned char *arr_buff, unsigned char len)
{
	uint crc=0xFFFF;
	int  i, j;
	for ( j=0; j<len;j++)
	{
		crc=crc ^*arr_buff++;
		for ( i=0; i<8; i++)
		{
			if( ( crc&0x0001) >0)
			{
				crc=crc>>1;
				crc=crc^ 0xa001;
			}
			else
				crc=crc>>1;
		}
	}
	return   crc;
}
