
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "cJSON.h"
#include "sqlite3.h"

#include "myMQTT.h"
#include "myUart.h"
#include "cQueue.h"
#include "print.h"
#include "global.h"
#include "info.h"
#include "hmiFSM.h"

extern const int gateway_id;
extern const int mqtt_qos;
extern char factory_name[128];
char  response_print_topic[256];

extern int UART_PTR_ID;
extern CQueue 		* qprintrecv;
extern pthread_mutex_t mutex_printrecvqueue;

void* th_print(void)
{
	CQNode	front;
	char	msg[SIZE_PRINT_MSG];
	int		msglen;
	int		ret;
	
	while(true)
	{
		if( !IsEmptyCQueue(qprintrecv) )
		{
			memset(msg,0,SIZE_PRINT_MSG);

			front	= qprintrecv->front;
			memcpy(msg,front->data,strlen(front->data));

			printf("th_print\n");
			pthread_mutex_lock(&mutex_printrecvqueue);
			DeCQueue(qprintrecv);
			pthread_mutex_unlock(&mutex_printrecvqueue);

			/* get gb2312 code */
			msglen	= strlen(msg)/2;
			getGB2312(msg,msg);

			printf("\nafter translate:\n");
			int ti2=0;
			for(ti2 = 0; ti2 < msglen; ti2++)
			{
				printf("%x ",msg[ti2]);
			}

			printf("msglen==%d\n",msglen);
			/*print function*/
			ret	= write(UART_PTR_ID,msg,msglen);
		}
		//avoid too much cpu occupation;
		usleep(10*1000);
	}

}
