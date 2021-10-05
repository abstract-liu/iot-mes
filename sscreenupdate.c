/*
 * sscreenupdate.c
 *
 *  Created on: Oct 15, 2020
 *      Author: lj
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <pthread.h>

#include "sscreenupdate.h"



extern int			UART_SCN_ID;
#define		POS_IMAGENAME	10
#define		DATA_LEN		2052
#define		SIZE_RET		128
#define		DATA_BLOCK2		2048
#define		DATA_BLOCK		498
#define		IMAGE_FILE		"3:\\33.jpg"
char		returnval[128];
extern pthread_mutex_t mutex_scnid;
int sn;


/*
 * image,sound file update
 *
 * */
int scn_update_path(unsigned char filedata[],int filelen,unsigned char path[],int baudrate)
{
	int i;

	/* cmd to start download */
	time_t tm = time(NULL);
	printf("path:%s,time:%d\n",IMAGE_FILE,tm);
	scn_update_cmd1(path,filelen,baudrate);

	usleep(600*1000);
	//scn_read_all();


	if(returnval[0] != 0xEE || returnval[1] != 0xFB ||
	   returnval[2] != 0x01 || returnval[3] != 0xFF ||
	   returnval[4] != 0xFC || returnval[5] != 0xFF || returnval[6] != 0xFF)
		return -1;

	usleep(11600*1000);
	/* packet divided to download */
	for(i=0;i*DATA_BLOCK<filelen;i = i + 1)
	{
		tm = time(NULL);
		printf("SN:%d and wait for %d microsec,time:%d\n",i,600,tm);

		sn = i;

		memset(returnval,0,128);

		int try = 10;
		

		if( (i+1)*DATA_BLOCK <= filelen )
			scn_update_cmd2(filedata+i*DATA_BLOCK,DATA_BLOCK,sn);
		else
			scn_update_cmd2(filedata+i*DATA_BLOCK,(i+1)*DATA_BLOCK-filelen,sn);

		try = 6;
		while(--try)
		{
			usleep(600*1000);
			if( returnval[0] != 0 || returnval[1] != 0 )
				break;
		}
		printf("Return value:\t");
		for(i=0;i<7;i++)
			printf(" %X",returnval[i]);
		printf("\n");

		if(returnval[0] != (i+1) || returnval[1] != ~(i+1) )
			return -2;
	}

	return 0;
}
unsigned short get_checksum(char*p,int size)
{
	int i;
	unsigned short checksum=0;

	for(i=0;i<size-2;i++)
		checksum += p[i];

	printf( "checksum and ~checksum:%4X, %4X\n",checksum,~checksum );

	return ~checksum;
}

//void add_crc16(UINT8 *data,UINT16 n,UINT16 *pcrc)
unsigned short get_checksum2(unsigned char*data,unsigned short n,unsigned short *pcrc)
{
    //UINT16 i,j,carry_flag,a;
    unsigned short i,j,carry_flag,a;

    for (i=0; i<n; i++)
    {
        *pcrc=*pcrc^data[i];
        for (j=0; j<8; j++)
        {
            a=*pcrc;
            carry_flag=a&0x0001;
            *pcrc=*pcrc>>1;
            if (carry_flag==1)
                *pcrc=*pcrc^0xa001;
        }
    }
}

int scn_update_cmd2(char data[],int datalen,int sn)
{
	unsigned char cmd[DATA_LEN]={0};
	unsigned short checksum=0;
	unsigned char*p;
	int i,len;

	//checksum	=	get_checksum(data,DATA_LEN);
	checksum	=	get_checksum(data,datalen+2);
	//checksum	=	crc16table(data,2050);
	//checksum 	= 0xffff;
	//get_checksum2( data,2050,&checksum );

	memset(cmd,0,DATA_LEN);

	cmd[0]	= (unsigned char)sn;
	cmd[1]	= (unsigned char)(~sn);

	memcpy(cmd+2,data,datalen);

	//memcpy(cmd+DATA_LEN-2,&checksum,2);
	p = (unsigned char*)&checksum;
	printf("Checksum:%X %X %X %X\n",p[0],p[1],p[2],p[3]);
	//memcpy(cmd+DATA_LEN-2,&checksum,2);
	cmd[datalen+2] =  p[0];
	cmd[datalen+3] =  p[1];

	write(UART_SCN_ID,cmd,datalen+4);

	for(i=0;i<datalen+4;i++)
		printf(" %X",cmd[i]);
	printf("\n");
	

	return 0;
}
int scn_update_cmd1(char path[],int filelen,int baudrate)
{
	/* EE FB */ 
	unsigned char cmd[1024]={0xEE, 0xFB, 0x00, 0x00,0x00, 0x00, 0x00,0x00,0x00,0x00,
		/*filename -13-Bytes*/
		0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,
		/*filename */
		0xFF, 0xFC, 0xFF, 0xFF};
	char	tail[4]={0xFF, 0xFC, 0xFF, 0xFF};
	unsigned char*tint=&filelen;
	int		realbaud;
	int		blocksize=2048+4;
	int		len = 14,i;


	/*block size*/
	tint		= &blocksize;
	cmd[POS_IMAGENAME-7] = tint[0];
	cmd[POS_IMAGENAME-8] = tint[1];
	/*baud rate*/
	if( baudrate == 0 )
	{
		cmd[POS_IMAGENAME-5] = 0;
		cmd[POS_IMAGENAME-6] = 0;
	}
	else
	{
		realbaud	= (baudrate/100);
		tint		= &realbaud;
		cmd[POS_IMAGENAME-5] = tint[0];
		cmd[POS_IMAGENAME-6] = tint[1];
	}
	/*filesize*/
	tint	=	&filelen;
	cmd[POS_IMAGENAME-1] = tint[0];
	cmd[POS_IMAGENAME-2] = tint[1];
	cmd[POS_IMAGENAME-3] = tint[2];
	cmd[POS_IMAGENAME-4] = tint[3];
	/*filename*/
	memcpy(cmd+POS_IMAGENAME,IMAGE_FILE,strlen(IMAGE_FILE));

	/* 0x00 0x00 fixed*/
	len = POS_IMAGENAME+strlen(IMAGE_FILE) ;
	cmd[len] 	= 0x00;
	/*CRC*/
	cmd[len+1] 	= 0x00;
	cmd[len+2] 	= 0x00;
	//cmd[len+3] 	= 0x00;
	unsigned short crc = crc16table(cmd+1,(unsigned short)len+2);
	memcpy(cmd+len+1,&crc,2);
	/*tail*/
	memcpy(cmd+len+3,tail,4);

	len += 7;

	write(UART_SCN_ID,cmd,len);

	printf("Update:\t");
	for(i=0;i<len;i++)
		printf(" %X",cmd[i]);
	printf("\n");

	return 0;
}

int scn_read_all()
{
	struct	timeval timeout = {3,0};		//select等待3秒，3秒轮询 sec usec
	fd_set	fs_read;
	int		i = 0,num = 0,n;
	int		length = 0;
	unsigned char *temp = returnval;


	if( 1 )
	{
		FD_ZERO(&fs_read);
		FD_SET(UART_SCN_ID,&fs_read);
		timeout.tv_sec	= 3;
		timeout.tv_usec = 0;

		if((n = select(UART_SCN_ID + 1,&fs_read,NULL,NULL,&timeout)) < 0)
		{
			printf("error in UartRXThr, error code = %d\n", n);
			perror("select");
			return -10;
		}
		else
		{
			if( FD_ISSET( UART_SCN_ID	,&fs_read) )
			{
				while( (num = read(UART_SCN_ID ,temp,SIZE_RET)) > 0 )
				{
					printf("Return value:\t");
					for(i=0;i<num;i++)
						printf("%02X ",temp[i]);
					printf("\n");
				}//end while
			}
		}
	}//end

	return -1;
}




 
uint16_t crc16bitbybit(uint8_t *ptr, uint16_t len)
{
	uint8_t i;
	uint16_t crc = 0xffff;
 
	if (len == 0) {
		len = 1;
	}
	while (len--) {
		crc ^= *ptr;
		for (i = 0; i<8; i++)
		{
			if (crc & 1) {
				crc >>= 1;
				crc ^= polynom;
			}
			else {
				crc >>= 1;
			}
		}
		ptr++;
	}
	return(crc);
}
 
uint16_t crc16table(uint8_t *ptr, uint16_t len)
{
	uint8_t crchi = 0xff;
	uint8_t crclo = 0xff; 
	uint16_t index;
	while (len--) 
	{
		index = crclo ^ *ptr++; 
		crclo = crchi ^ crctablehi[index];
		crchi = crctablelo[index];
	}
	return (crchi << 8 | crclo);
}
 
const uint16_t crctalbeabs[] = { 
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 
	0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 
};
 
uint16_t crc16tablefast(uint8_t *ptr, uint16_t len) 
{
	uint16_t crc = 0xffff; 
	uint16_t i;
	uint8_t ch;
 
	for (i = 0; i < len; i++) {
		ch = *ptr++;
		crc = crctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
		crc = crctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
	} 
	
	return crc;
}
 
void modbuscrc16test()
{
	printf("\n");
	printf(" Modbus CRC16 tester\n");
	printf("-----------------------------------------------------------------------\n");
	uint8_t crc16_data[] = { 0x01, 0x04, 0x04, 0x43, 0x6b, 0x58, 0x0e };	// expected crc value 0xD825.
	printf(" modbus crc16table test, expected value : 0xd825, calculate value : 0x%x\n", crc16table(crc16_data, sizeof(crc16_data)));
	printf(" modbus crc16tablefast test, expected value : 0xd825, calculate value : 0x%x\n", crc16tablefast(crc16_data, sizeof(crc16_data)));
	printf(" modbus crc16bitbybit test, expected value : 0xd825, calculate value : 0x%x\n", crc16bitbybit(crc16_data, sizeof(crc16_data)));
}
