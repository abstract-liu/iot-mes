/*
 * request_print.c
 *
 *  Created on: Oct 15, 2020
 *      Author: lj
 */

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>

#include "mosquitto.h"

#include "myMQTT.h"
#include "cJSON.h"
#include "sqlite3.h"

#include "hmiFSM.h"
#include "myUart.h"
#include "global.h"
#include "info.h"
#include "CRC.h"
#include "cQueue.h"
#include "myQueue.h"
#include "serialscreen.h"



#define SIZE_RFIDBUF		128
#define RFID_HEADER_LEN		5
#define RFID_TAILER_LEN		1
#define RFID_TYPE_LEN		2	



//extern char			*topic_serialscreen;
//extern char			*topic_mes_send;
extern char			factory_name[128];
extern int			gateway_id;
//extern int			mqtt_qos;
extern int			UART_SCN_ID;

extern Queue 		* qrfid;		
extern Queue 		* qscreensend;
extern Queue 		* qscreenrecv;
extern CQueue 		* qmqttsend;
extern CQueue 		* qmqttrecv;

extern pthread_mutex_t mutex_rfidqueue;		
extern pthread_mutex_t mutex_scnsendqueue;
extern pthread_mutex_t mutex_scnrecvqueue;
extern pthread_mutex_t mutex_mqttrecvqueue;
extern pthread_mutex_t mutex_mqttsendqueue;
extern pthread_mutex_t mutex_scnid;

extern char		returnval[128];
extern int		sn;	

void th_serialscreen_recv(void)
{
	int		i=0,j=1,num;
	int		n;
	int		cnt=1,remain;
	//char	name[10];
	//int		type,j = 1;
	//int		i = 0,num = 0,flag = 0,num_clear = 0;
	//char	str[30];
	//int ii=0,flag_matching=0;
	//char	str_comp[30];
	//int		print_flag=0;
	//int flag_start=0,flag_mid=0,flag_end=1;
	//unsigned char valid[100];
	//unsigned char array[128];
	//int frame_len=0;
	//unsigned char rfid[128];
	//int common_type_flag=0;//1是读卡器，协议2是屏协议
	//unsigned char scn[128];

	struct	timeval timeout = {3,3000};
	fd_set	fs_read;
	unsigned char temp[128];
	unsigned char frame[128];
	struct ComFrame	cf;
	int	rfidlen = 0;

	Itemt	itm = &cf;
	Itemt1	itm1;

	unsigned char *rtv = returnval;


	while( 1 )
	{
		/*reset fd */
		FD_ZERO(&fs_read);
		FD_SET(UART_SCN_ID,&fs_read);

		timeout.tv_sec	= 3;
		timeout.tv_usec = 0;

		if((n = select(UART_SCN_ID+1,&fs_read,NULL,NULL,&timeout)) < 0 )
		{
			perror("select");
			continue;
		}
		else if(n == 0)
		{
			//printf("Timeout of 3 second\n");
			continue;
		}
		else
		{
			if( FD_ISSET(UART_SCN_ID,&fs_read) )
			{
				//cnt = 100*100;
				//while(cnt--);
				usleep(50*1000);

				remain = 0;

				while((num = read(UART_SCN_ID,temp+remain,128-remain)) > 0)//read to temp
				{
					num += remain;

				//	printf("SN recv:%X | %X\t",sn,~sn);
					for(i = 0;i<num;i++)
						printf(" %X",temp[i]);
					printf("\n");

					/* pickup a total packet */
					for(i = 0;i<num;i++)
					{
						/* rfid frame */
						if( temp[i] == 0x04 )//判断是读卡器，且是帧头
						{
							if( num > i+1 && ((num-i) < temp[i+1]) )
							{
								printf("Rfid data error: wrong length\n");
								break;
							}
							else if( rfid_checksum(temp+i) != temp[temp[i+1] - 1] )
							{
								printf("Rfid data error: wrong checksum\n");
								break;
							}
							else
							{
								rfidlen		= temp[i+1] - RFID_HEADER_LEN - RFID_TYPE_LEN - RFID_TAILER_LEN;
								itm->frmlen = rfidlen;
								memcpy(itm->data,temp+i+RFID_HEADER_LEN+RFID_TYPE_LEN,rfidlen);

								pthread_mutex_lock(&mutex_rfidqueue);
								EnQueue(qrfid,itm);
								pthread_mutex_unlock(&mutex_rfidqueue);
								break;
							}
						}
						/* serial screen update cmd frame return */
						else if( temp[i] == 0xEE && temp[i+1] == 0xFB )
						{
							time_t tm = time(NULL);
							printf("serial screen update cmd frame return,time:%d\n",tm);
							memcpy(rtv,temp,num);
						}
						/* serial screen update data frame return */
						else if(  (temp[i]  == sn && temp[i+1] == (unsigned char)~sn)|| (temp[i]  == sn+1 && temp[i+1] == (unsigned char)~(sn+1)) )
						{
							time_t tm = time(NULL);
							printf("serial screen update data frame return,time:%d\n",tm);
							memcpy(rtv,temp,num);
						}
						/* serial screen frame */
						else if( temp[i] == 0xEE )
						{
							j = i;
							while( ( num-j>4 ) && j<num &&
								   ( (temp[j+1] != 0xFF) || (temp[j+2] != 0xFC) ||\
								     (temp[j+3] != 0xFF) || (temp[j+4] != 0xFF) ) )j++;

							if( ( num-j>4 ) && j<num &&
								   ( (temp[j+1] == 0xFF) && (temp[j+2] == 0xFC) &&\
									 (temp[j+3] == 0xFF) && (temp[j+4] == 0xFF) ))
							{
								itm->frmlen = j-i+4 + 1;
								memcpy(itm->data,temp+i,itm->frmlen);

								printf("frame len: %d\n",itm->frmlen);

								pthread_mutex_lock(&mutex_scnrecvqueue);
								EnQueue(qscreenrecv,itm);
								pthread_mutex_unlock(&mutex_scnrecvqueue);
								/* 
								 * remain = num - j - 4 - 1
								 * if(remain > 0 )
								 * memcpy(temp,temp+(j+4+1),remain);
								 * */
								break;
							}
						}
					}// temp for end
				}// while read end 
			}//end if isset
		}//else end
	}//while(1) pthread  end
}
/* send the queue data to serial screen */
void th_serialscreen_send(void)
{
	int i;
	QNode	front;
	struct	ComFrame cf;	

	while(true)
	{
		/* send all the command in screen queue */
		if( !IsEmptyQueue(qscreensend))
		{
			front = qscreensend->front;

			cf.frmlen	= front->data->frmlen;
			memcpy(cf.data,front->data->data,front->data->frmlen);

			pthread_mutex_lock(&mutex_scnsendqueue);
			DeQueue(qscreensend);
			pthread_mutex_unlock(&mutex_scnsendqueue);

			write(UART_SCN_ID,cf.data,cf.frmlen);

			printf("Send Thread:\t");
			for(i = 0;i < cf.frmlen;i++)
				printf(" %X",cf.data[i]);
			printf("\n");
		}

		//avoid too much cpu occupation;
		usleep(10*1000);
	}
}

int scn_cmd_switch(int scnid)
{
	/*B1 00*/ 
	char cmd[16]={0xEE, 0xB1, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 9;

	/* screen id */
	cmd[POS_SCN]	= scnid>>8;
	cmd[POS_SCN+1]	= scnid&0x00FF;

	write(UART_SCN_ID,cmd,len);

	return 0;
}

int scn_cmd_realtime(int scnid)
{
	/*81 00*/ 
	char cmd[16]={0xEE, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 13;

	time_t nSeconds;
	struct tm * pTM;

	time(&nSeconds);
	pTM = localtime(&nSeconds);

	/* screen id */
	cmd[POS_SEC]	= pTM->tm_sec;
	cmd[POS_SEC+1]	= pTM->tm_min;
	cmd[POS_SEC+2]	= pTM->tm_hour;

	cmd[POS_SEC+3]	= pTM->tm_mday;
	cmd[POS_SEC+4]	= pTM->tm_wday;
	cmd[POS_SEC+5]	= pTM->tm_mon;
	cmd[POS_SEC+6]	= pTM->tm_year;

	write(UART_SCN_ID,cmd,len);

	return 0;
}

int scn_cmd_setctl(int scnid,int ctlid,char*content,int contentlen)
{
	/*B1 10*/ 
	char cmd[1024]={0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x1E, 0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	char tail[4]={0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 11;

	/* screen id */
	cmd[POS_SCN]	= scnid>>8;
	cmd[POS_SCN+1]	= scnid&0x00FF;
	/* control id */
	cmd[POS_CTL]	= ctlid>>8;
	cmd[POS_CTL+1]	= ctlid&0x00FF;
	/* command content */
	memcpy(cmd+POS_CMD,content,contentlen);
	/* tail */
	len = POS_CMD + contentlen;
	memcpy(cmd+POS_CMD+len,tail,4);

	len	+= 4;
	write(UART_SCN_ID,cmd,len);

	return 0;
}

int scn_cmd_flash(int scnid,int ctlid,int cycle)
{
	char cmd[16]={0xEE, 0xB1, 0x15, 0x00, 0x04, 0x00, 0x07, 0x00,0x0A, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 13;

	/* screen id */
	cmd[POS_SCN]	= scnid>>8;
	cmd[POS_SCN+1]	= scnid&0x00FF;
	/* control id */
	cmd[POS_CTL]	= ctlid>>8;
	cmd[POS_CTL+1]	= ctlid&0x00FF;
	/* flash cycle*/
	cmd[POS_CMD]	= cycle>>8;
	cmd[POS_CMD+1]	= cycle&0x00FF;

	write(UART_SCN_ID,cmd,len);

	return 0;
}
int scn_cmd_buzzeron(int time)
{
	char cmd[16]={0xEE, 0x61, 0x0A, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 7;

	/* time long*/
	cmd[2]	= time;

	write(UART_SCN_ID,cmd,len);

	return 0;
}
int scn_cmd_reset()
{
	char cmd[16]={0xEE, 0x07, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 6;

	write(UART_SCN_ID,cmd,len);

	return 0;
}
int scn_cmd_visible(int scnid,int ctlid,int visible)
{// B1 03
	char cmd[16]={0xEE, 0xB1, 0x03, 0x00, 0x00, 0x00, 0x1E, 0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 12;

	/* screen id */
	cmd[POS_SCN]	= scnid>>8;
	cmd[POS_SCN+1]	= scnid&0x00FF;
	/* control id */
	cmd[POS_CTL]	= ctlid>>8;
	cmd[POS_CTL+1]	= ctlid&0x00FF;
	/* flag */
	cmd[POS_CMD]	= visible&0x00FF;

	write(UART_SCN_ID,cmd,len);

	return 0;
}
int scn_cmd_emptyrecord(int scnid,int ctlid)
{//B1 53
	char cmd[16]={0xEE, 0xB1, 0x53, 0x00, 0x00, 0x00, 0x1E, 0xFF, 0xFC, 0xFF, 0xFF};
	int	 len = 11;

	/* screen id */
	cmd[POS_SCN]	= scnid>>8;
	cmd[POS_SCN+1]	= scnid&0x00FF;
	/* control id */
	cmd[POS_CTL]	= ctlid>>8;
	cmd[POS_CTL+1]	= ctlid&0x00FF;

	write(UART_SCN_ID,cmd,len);

	return 0;
}
//int scn_cmd_setsinglerecord(int scnid,int ctlid,int recordid,char*content,int contentlen)
int scn_cmd_setsinglerecord(int scnid,int ctlid,struct MQTTReturn*mr,int recordid)
{//B1 57
	char cmd[1024]={0xEE, 0xB1, 0x57, 0x00, 0x00, 0x00, 0x1E, 0xFF, 0xFC, 0xFF, 0xFF};
	char tail[4]={0xFF, 0xFC, 0xFF, 0xFF};
	//int	recordNum = mr->curTaskNum;
	int len,iv=0;
	int	record_size;	

	/* screen id */
	cmd[POS_SCN]	= (char)scnid>>8;
	cmd[POS_SCN+1]	= (char)scnid&0x00FF;
	/* control id */
	cmd[POS_CTL]	= (char)ctlid>>8;
	cmd[POS_CTL+1]	= (char)ctlid&0x00FF;
	/* record num */
	cmd[POS_RCD]	= (char)recordid>>8;
	cmd[POS_RCD+1]	= (char)recordid&0x00FF;

	/* command len*/
	len	= POS_RCD+2;

	for(iv=recordid;iv<recordid+1;iv++)
	{
		/* record size
		record_size	= 2+\
					  strlen(mr->td[iv]->taskId)+1+\
					  strlen(mr->td[iv]->norms)+1+\
					  strlen(mr->td[iv]->totalWeight)+strlen(mr->td[iv]->complishWeight)+1+\
					  strlen(mr->td[iv]->status)+1; */

		/* lenght of each column with 2-bytes 
		cmd[len]	= (char)record_size>>8;
		cmd[len+1]	= (char)record_size&0xFF;
		len			+= 2;*/

		/* 1st column number */;
		sprintf(cmd+len,"%c%c",(char)iv+'0',0x3B);
		len			+= 2;
		/* 2nd column number taskId*/
		sprintf(cmd+len,"%s%c",mr->td[iv]->instructionOrder,0x3B);
		len			+= strlen(mr->td[iv]->instructionOrder)+1;

		/* 3rd column number norms */
		sprintf(cmd+len,"%s%c",mr->td[iv]->norms,0x3B);
		len			+= strlen(mr->td[iv]->norms)+1;

		/* 4th column number other requirement*/
		sprintf(cmd+len,"%s%c%s%c%s%c",mr->td[iv]->wireSize,0x2f,mr->td[iv]->complishWeight,0x2f,mr->td[iv]->totalWeight,0x3B);
		len			+= strlen(mr->td[iv]->wireSize);
		len			+= strlen(mr->td[iv]->totalWeight);
		len			+= strlen(mr->td[iv]->complishWeight)+3;

		/* 5th column number status*/
		printf("%s\n",mr->td[iv]->status);
		sprintf(cmd+len,"%s%c",mr->td[iv]->status,0x3B);// there not have 0x3B
		len			+= strlen(mr->td[iv]->status)+1;
	}

	/* tail */
	memcpy(cmd+len,tail,4);
	len	+= 4;

	write(UART_SCN_ID,cmd,len);

	int jj=0;
	for(jj=0;jj<len;jj++)
		printf(" %x",cmd[jj]);
	printf("\n");
	return 0;
}
int scn_cmd_setmultiplerecord(int scnid,int ctlid,struct MQTTReturn*mr)
{//B1 5B
	char cmd[1024]={0xEE, 0xB1, 0x5B, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFC, 0xFF, 0xFF};
	char tail[4]={0xFF, 0xFC, 0xFF, 0xFF};
	int	recordNum = mr->curTaskNum;
	int len=0,iv=0;
	int record_size = 0;
	int page;

	/* screen id */
	cmd[POS_SCN]	= (char)scnid>>8;
	cmd[POS_SCN+1]	= (char)scnid&0x00FF;
	/* control id */
	cmd[POS_CTL]	= (char)ctlid>>8;
	cmd[POS_CTL+1]	= (char)ctlid&0x00FF;
	/* record id */
	cmd[POS_RCD]	= (char)recordNum>>8;
	cmd[POS_RCD+1]	= (char)recordNum&0x00FF;
	/* command len */
	len	= POS_RCD+2;

	/* command content */
	len	= POS_LEN;
	for(iv=0;iv<recordNum;iv++)
	{
		printf("\npage is:%d\trecordNum: %d\t%s\t%s\t%s\t%s\n",mr->page,recordNum,(mr->td[iv])->taskId,(mr->td[iv])->norms,(mr->td[iv])->totalWeight,(mr->td[iv])->status);
		/* record size */;
		record_size	= 2+\
					  strlen(mr->td[iv]->instructionOrder)+1+\
					  strlen(mr->td[iv]->norms)+1+\
					  strlen(mr->td[iv]->wireSize)+1+\
					  strlen(mr->td[iv]->totalWeight)+1+\
					  strlen(mr->td[iv]->complishWeight)+1+\
					  strlen(mr->td[iv]->status)+1;

		/* lenght of each column with 2-bytes */;
		cmd[len]	= (char)record_size>>8;
		cmd[len+1]	= (char)record_size&0xFF;
		len			+= 2;

		/* 1st column number */;
		//page		= ( (mr->page) - 1 )*5+iv;
		//itoa(page, cmd+len, 10);
		//len			+= page>9?page>99?3:2:1;
		sprintf(cmd+len,"%c%c",'0'+ iv,0x3B);
		len			+= 2;
		//sprintf(cmd+len,"%s%c",mr->ud.realName,0x3B);
		//len			+= strlen(mr->ud.realName)+1;
		/* 2nd column taskId*/
		sprintf(cmd+len,"%s%c",mr->td[iv]->instructionOrder,0x3B);
		len			+= strlen(mr->td[iv]->instructionOrder)+1;

		/* 3rd column norms */
		sprintf(cmd+len,"%s%c",mr->td[iv]->norms,0x3B);
		len			+= strlen(mr->td[iv]->norms)+1;

		/* 4th column other requirements*/
		sprintf(cmd+len,"%s%c%s%c%s%c",mr->td[iv]->wireSize,0x2F,mr->td[iv]->complishWeight,0x2F,mr->td[iv]->totalWeight,0x3B);
		len			+= strlen(mr->td[iv]->wireSize)+1;
		len			+= strlen(mr->td[iv]->totalWeight)+1;
		len			+= strlen(mr->td[iv]->complishWeight)+1;
		/* 5th column status*/
		sprintf(cmd+len,"%s%c",mr->td[iv]->status,0x3B);
		len			+= strlen(mr->td[iv]->status)+1;
	}

	/* tail */
	memcpy(cmd+len,tail,4);
	len	+= 4;

	write(UART_SCN_ID,cmd,len);


	return 0;
}

int scn_cmd_seticon(int scnid,int ctlid,int iconid)
{
	char cmd[1024]={0xEE, 0xB1, 0x23, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	char tail[4]={0xFF, 0xFC, 0xFF, 0xFF};

	int len=0,iv=0;
	int record_size = 0;

	/* scn id */
	cmd[POS_SCN]	= (char)scnid>>8;
	cmd[POS_SCN+1]	= (char)scnid&0x00FF;

	/* ctl id */
	cmd[POS_CTL]	= (char)ctlid>>8;
	cmd[POS_CTL+1]	= (char)ctlid&0x00FF;


	/* icon num*/
	cmd[POS_RCD]	= (char)iconid;

	len		=	POS_RCD + 1;


	memcpy(cmd+len,tail,4);
	len		+= 4;


	write(UART_SCN_ID,cmd,len);

	return 0;

}
int scn_cmd_opmusic(int musicid,int op)
{
	char cmd[1024]={0xEE, 0x90, 0x01, 0x00, 0x00, 0x01,  0xFF, 0xFC, 0xFF, 0xFF};
	char pause[16]={0xEE, 0x90, 0x02,                    0xFF, 0xFC, 0xFF, 0xFF};

	int len=0,iv=0;
	int record_size = 0;

	printf("musicid==%d,opid==%d\n",musicid,op);

	/* content */
	if(op = 1)
	{
		cmd[POS_MUSICID+1]	= (char)musicid&0x00FF;
		write(UART_SCN_ID,cmd,10);
	}
	else
	{
		write(UART_SCN_ID,pause,7);
	}

	return 0;
}


int scn_cmd_settextp(int scnid,int ctlid,char *p)
{
	char cmd[1024]={0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	char tail[4]={0xFF, 0xFC, 0xFF, 0xFF};

	int len=0,iv=0;
	int record_size = 0;

	/* scn id */
	cmd[POS_SCN]	= (char)scnid>>8;
	cmd[POS_SCN+1]	= (char)scnid&0x00FF;

	/* ctl id */
	cmd[POS_CTL]	= (char)ctlid>>8;
	cmd[POS_CTL+1]	= (char)ctlid&0x00FF;


	/* content */
	memcpy(cmd+POS_RCD,p,strlen(p));

	len		=	POS_RCD + strlen(p);


	memcpy(cmd+len,tail,4);
	len		+= 4;


	write(UART_SCN_ID,cmd,len);

	return 0;
}

int scn_cmd_settext(int scnid,int ctlid,struct MQTTReturn*mr)
{
	char cmd[1024]={0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC, 0xFF, 0xFF};
	char tail[4]={0xFF, 0xFC, 0xFF, 0xFF};

	printf("scnid==%d,ctlid==%d\n",scnid,ctlid);
	int len=0,iv=0;
	int record_size = 0;

	/* scn id */
	cmd[POS_SCN]	= (char)scnid>>8;
	cmd[POS_SCN+1]	= (char)scnid&0x00FF;

	/* ctl id */
	cmd[POS_CTL]	= (char)ctlid>>8;
	cmd[POS_CTL+1]	= (char)ctlid&0x00FF;

	printf("realName==%s\n",mr->ud.realName);

	/* name */
	//sprintf(cmd+POS_RCD,"%s",mr->ud.realName);
	memcpy(cmd+POS_RCD,mr->ud.realName,strlen(mr->ud.realName));

	len		=	POS_RCD + strlen(mr->ud.realName);


	memcpy(cmd+len,tail,4);
	len		+= 4;

	int i=0;
	for(i = 0;i<len;i++)
		printf(" %x",cmd[i]);
	printf("\n");

	write(UART_SCN_ID,cmd,len);

	return 0;
}
