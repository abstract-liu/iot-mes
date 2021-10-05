/*
 * rfid_recv.c
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
#include "rfid.h"



#define SIZE_RFIDBUF		128
#define RFID_HEADER_LEN		5
#define RFID_TAILER_LEN		1
#define RFID_TYPE_LEN		2	



extern int			gateway_id;
extern int			UART_RFID_ID;

extern Queue 		* qrfid;		

extern pthread_mutex_t mutex_rfidqueue;		


void *th_rfid_recv(void)
{
	int		i=0,j=1,num;
	int		n;
	int		cnt=1,remain;
	struct	timeval timeout = {3,3000};
	fd_set	fs_read;
	unsigned char temp[128];
	unsigned char frame[128];
	struct ComFrame	cf;
	int	rfidlen = 0;

	Itemt	itm = &cf;
	Itemt1	itm1;



	while( 1 )
	{
		/*reset fd */
		FD_ZERO(&fs_read);
		FD_SET(UART_RFID_ID,&fs_read);

		timeout.tv_sec	= 3;
		timeout.tv_usec = 0;

		if((n = select(UART_RFID_ID+1,&fs_read,NULL,NULL,&timeout)) < 0 )
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
			if( FD_ISSET(UART_RFID_ID,&fs_read) )
			{
				//cnt = 100*100;
				//while(cnt--);
				usleep(50*1000);

				remain = 0;

				while((num = read(UART_RFID_ID,temp+remain,128-remain)) > 0)//read to temp
				{
					num += remain;

					for(i = 0;i<num;i++)
						printf(" %x",temp[i]);
					printf("\n");

					/* pickup a total rfid packet */
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
						/* other rfid card type */
					}// temp for end
				}// while read end 
			}//end if isset
		}//else end
	}//while(1) pthread  end
}

