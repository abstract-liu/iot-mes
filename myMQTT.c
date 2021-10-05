
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "cJSON.h"
#include "mosquitto.h"

#include "cQueue.h"
#include "myQueue.h"
#include "info.h"

#include "hmiFSM.h"
#include "myMQTT.h"

char topic_none[256]={"qwert"};

/* MQTT变量 */
const int			mqtt_qos = 0;
const int			mqtt_keep_alive = 60;
const extern int	mqtt_port;
const extern char	mqtt_host[50];

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

char *functionlist[16]={"login","cardlogin","getDeviceTask","taskStart","taskPause",\
					    "taskGoon","taskEnd","print","taskPrev","taskNext",\
						"taskDetail","facelogin"};

void serialscreen_callback(struct mosquitto *mosq, void *userdata, \
		const struct mosquitto_message *msg);
void serialscreen_cnt_callback(struct mosquitto *mosq, void *userdata, int result);
void serialscreen_cnt_callback1(struct mosquitto *mosq, void *userdata, int result);
void send_disconnect_callback(struct mosquitto *mosq, void *userdata, int result);


void mqtt_send_cmd(int function,char*gatewayid,char*user,char*pass,
			  	  char*employid,char rfid[],char *taskid ,int page)
{
	char		rfid_asci[16];
	time_t	t;
	char *	msg_pub;
	cJSON *	root;
	int		timetick	= time(&t);

	root	= cJSON_CreateObject();


	switch( function )
	{
		case USRLOGIN:
			printf("USRLOGIN\n");
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[USRLOGIN]) );
			cJSON_AddItemToObject(root, "userAccount", cJSON_CreateString(user));
			cJSON_AddItemToObject(root, "password", cJSON_CreateString(pass));
			break;
		case RFIDLOGIN:
			memset(rfid_asci,0,16);
			sprintf(rfid_asci, "%02X%02X%02X%02X",rfid[0],rfid[1],rfid[2],rfid[3]);
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[RFIDLOGIN]) );
			cJSON_AddItemToObject(root, "rfid", cJSON_CreateString(rfid_asci));
			break;
		case FACELOGIN:
			printf("FACELOGIN\n");
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[FACELOGIN]) );
			cJSON_AddItemToObject(root, "faceid", cJSON_CreateString(user));
			cJSON_AddItemToObject(root, "userAccount", cJSON_CreateString(user));
			break;
		case TASKGET:
			printf("TASKGET\n");
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKGET]) );
			cJSON_AddItemToObject(root, "employId", cJSON_CreateString(employid));
			cJSON_AddItemToObject(root, "index", cJSON_CreateNumber(page));
			//cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			break;
		case TASKSTART:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKSTART]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			break;
		case TASKPAUSE:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKPAUSE]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			break;
		case TASKGOON://depricated
			break;
		case TASKEND:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKEND]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			break;
		case TASKPRINT:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKPRINT]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			break;
		case TASKPREV:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKPREV]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			cJSON_AddItemToObject(root, "index", cJSON_CreateNumber(page));
			break;
		case TASKNEXT:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKNEXT]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			cJSON_AddItemToObject(root, "index", cJSON_CreateNumber(page));
			break;
		case TASKDETAIL:
			cJSON_AddItemToObject(root, "function", cJSON_CreateString( functionlist[TASKDETAIL]) );
			cJSON_AddItemToObject(root, "taskId",cJSON_CreateString(taskid) );
			break;
		default:
			break;
	}

	//cJSON_AddItemToObject(root, "gatewayId", cJSON_CreateNumber(gatewayid));
	//cJSON_AddItemToObject(root, "sequence", cJSON_CreateNumber(timetick));
	cJSON_AddItemToObject(root, "gatewayId", cJSON_CreateString(gatewayid));
	cJSON_AddItemToObject(root, "sequence", cJSON_CreateNumber(timetick));

	msg_pub = cJSON_Print(root);

	/* msg send by EnCQueue */
	printf("msg_pub==%s\n",msg_pub);

	pthread_mutex_lock(&mutex_mqttsendqueue);
	EnCQueue(qmqttsend,msg_pub);
	pthread_mutex_unlock(&mutex_mqttsendqueue);

	/* 清除cJSON实体 */
	cJSON_Delete(root);
	free(msg_pub);
	return;
}



void th_mqtt_real_send(void*arg)
{
	/* mqtt data structure */
	struct	mosquitto *mosq;

	/*  other data structure */
	CQNode	front;
	char	msg[SIZE_MQTT_MSG ];

	mosq	= (struct	mosquitto *)arg;

	while(true)
	{
		/* send all the command in screen queue */
		if( !IsEmptyCQueue(qmqttsend) )
		{
			front = qmqttsend->front;

			memset(msg,0,SIZE_MQTT_MSG);
			memcpy(msg,front->data,strlen(front->data));

			pthread_mutex_lock(&mutex_mqttsendqueue);
			DeCQueue(qmqttsend);
			pthread_mutex_unlock(&mutex_mqttsendqueue);

			//printf("topic_send:%s with msg:%s\n",topic_send,msg);

			mqtt_pub(mosq, topic_send,msg);
		}

		//avoid too much cpu occupation;
		usleep(10*1000);
	}


}
/* send the queue data to microMES */
void th_mqtt_send(void)
{
	/* mqtt data structure */
	struct	mosquitto *mosq,*arg;

	/*  other data structure */
	CQNode	front;
	pthread_t	th_mqtt_real_send_id;
	int err;


	mosq	= mqtt_init(NULL, serialscreen_cnt_callback1);
	mosquitto_threaded_set(mosq,1);

	
	arg		= mosq;
	err = pthread_create(&th_mqtt_real_send_id, NULL, (void*) th_mqtt_real_send, (void*)arg);
	if(err != 0)
	{
		printf("can't create th_print_id: %s\n",strerror(err));
		exit(-1);
	}

	mqtt_sub(mosq, topic_none);

	mqtt_quit(mosq);
	pthread_detach(pthread_self());
}

/* send the queue data to microMES */
void th_mqtt_recv(void)
{
	printf("th_mqtt_recv start\n");
	struct mosquitto *mosq;

	mosq = mqtt_init(serialscreen_callback, serialscreen_cnt_callback);
	mosquitto_threaded_set(mosq,1);

	mqtt_sub(mosq, topic_serialscreen);
	mqtt_quit(mosq);

	pthread_detach(pthread_self());
}

/*  callback function for MQTT recv */
void serialscreen_callback(struct mosquitto *mosq, void *userdata,
		const struct mosquitto_message *msg)
{
	char tmsg[SIZE_PAYLOAD];

	Itemt1	itm;


	memset(tmsg,0,SIZE_PAYLOAD);
	memcpy(tmsg,msg->payload,strlen(msg->payload));

	itm = tmsg;


	pthread_mutex_lock(&mutex_mqttrecvqueue);
	EnCQueue(qmqttrecv,itm);
	pthread_mutex_unlock(&mutex_mqttrecvqueue);

	printf("serialscreen_cnt_callback end\n");
}

/* 断线重连实现 */
void serialscreen_cnt_callback(struct mosquitto *mosq, void *userdata, int result)
{
	printf("serialscreen_cnt_callback reconnect\n");
	if (!result)
	{
		/* 重新订阅MQTT消息 */
		mosquitto_subscribe(mosq, NULL, topic_serialscreen, mqtt_qos);
	}
	else
	{
		printf("Connect failed\n");
	}
}

/* 断线重连实现 */
void serialscreen_cnt_callback1(struct mosquitto *mosq, void *userdata, int result)
{
	printf("serialscreen_cnt_callback reconnect\n");
	if (!result)
	{
		/* 重新订阅MQTT消息 */
		mosquitto_subscribe(mosq, NULL, topic_send, mqtt_qos);
	}
	else
	{
		printf("Connect failed\n");
	}
}



struct mosquitto* mqtt_init(void *message_callback, void *connect_callback)
{
	struct mosquitto *mosq;

	/* libmosquitto 库初始化 */
	mosquitto_lib_init();

	/* 创建mosquitto客户端 */
	mosq = mosquitto_new(NULL, 1, NULL);
	if (!mosq)
	{
		printf("create client failed..\n");
		mosquitto_lib_cleanup();
		return NULL;
	}

	mosquitto_message_callback_set(mosq, message_callback);
	mosquitto_connect_callback_set(mosq, connect_callback);

	while (mosquitto_connect(mosq, mqtt_host, mqtt_port, mqtt_keep_alive))
	{
		printf("host: %s, port: %d\n", mqtt_host, mqtt_port);
		fprintf(stderr, "Unable to connect...Reconnect...\n");
		sleep(2);
	}
	printf("MQTT broker %s connected on port %d.\n", mqtt_host, mqtt_port);

	return mosq;
}

int mqtt_quit(struct mosquitto *mosq)
{
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}

int mqtt_pub(struct mosquitto *mosq, const char *topic, const char *msg)
{
	int rc;
	rc = mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, mqtt_qos, 0);

	if (rc != MOSQ_ERR_SUCCESS)
	{
		printf("mosquitto_publish failed, topic:%s, msg: %s, error code: %d\n",
				topic, msg, rc);
		return 1;
	}

	return 0;
}

void mqtt_sub(struct mosquitto *mosq, const char *topic)
{
	mosquitto_subscribe(mosq, NULL, topic, mqtt_qos);
	mosquitto_loop_forever(mosq, -1, 1);
}

void mqtt_sub_multiple(struct mosquitto *mosq, int topic_count,
		char *const*const topics)
{
	mosquitto_subscribe_multiple(mosq, NULL, topic_count, topics, mqtt_qos, 0,NULL);
	mosquitto_loop_forever(mosq, -1, 1);
}

int	get_jsondata(MQTTReturn *mr,char*msg)
{
	cJSON *root,*data,*subdata;
	char		function[64];
	int			taskNum,iCnt,iv;

	printf("get_jsondata:%s\n",msg);

	root = cJSON_Parse((const char*) msg);

	if (NULL == root)
	{
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		return;
	}

	if (strcmp(cJSON_GetObjectItem(root, "gatewayId")->valuestring,mac) != 0 )
	{
		printf("id===%d\n",cJSON_GetObjectItem(root, "gatewayId")->valuestring);
		return;
	}


	//sprintf(&(mr->status), "%d", cJSON_GetObjectItem(root, "status")->valuestring);
	if(cJSON_GetObjectItem(root, "status") != NULL)	
		sscanf( cJSON_GetObjectItem(root, "status")->valuestring,"%d",&(mr->status));
	sprintf((mr->sequence), "%s", cJSON_GetObjectItem(root, "sequence")->valuestring);

	sprintf(function, "%s", cJSON_GetObjectItem(root, "function")->valuestring);

	if (strcmp(function, "cardlogin") == 0)
	{
		mr->function	=	RFIDLOGIN;
		data		=   cJSON_GetObjectItem(root, "data");
		if( mr->status == MQTT_OK_STATUS && data != NULL)
		{
			memset(mr->ud.userAccount,0,64);
			memset(mr->ud.employId,0,64);
			memset(mr->ud.realName,0,64);

			sprintf( (mr->ud.userAccount), "%s", cJSON_GetObjectItem(data, "userAccount")->valuestring);
			sprintf( (mr->ud.employId), "%s", cJSON_GetObjectItem(data, "employId")->valuestring);

			//sprintf( (mr->ud.realName), "%s", cJSON_GetObjectItem(data, "realName")->valuestring);
			getGB2312(cJSON_GetObjectItem(data, "realName")->valuestring,mr->ud.realName );
		}
	}
	else if (strcmp(function, "login") == 0)
	{
		mr->function	=	USRLOGIN;

		data		=   cJSON_GetObjectItem(root, "data");
		if( mr->status == MQTT_OK_STATUS && data != NULL)
		{
			memset(mr->ud.userAccount,0,64);
			memset(mr->ud.employId,0,64);
			memset(mr->ud.realName,0,64);

			sprintf( (mr->ud.userAccount), "%s", cJSON_GetObjectItem(data, "userAccount")->valuestring);
			sprintf( (mr->ud.employId), "%s", cJSON_GetObjectItem(data, "employId")->valuestring);

			//sprintf( (mr->ud.realName), "%s", cJSON_GetObjectItem(data, "realName")->valuestring);
			getGB2312(cJSON_GetObjectItem(data, "realName")->valuestring,mr->ud.realName );
		}
	}
	else if (strcmp(function, "facelogin") == 0)
	{
		mr->function	=	FACELOGIN;

		data		=   cJSON_GetObjectItem(root, "data");
		if( mr->status == MQTT_OK_STATUS && data != NULL)
		{
			memset(mr->ud.userAccount,0,64);
			memset(mr->ud.employId,0,64);
			memset(mr->ud.realName,0,64);

			sprintf( (mr->ud.userAccount), "%s", cJSON_GetObjectItem(data, "userAccount")->valuestring);
			sprintf( (mr->ud.employId), "%s", cJSON_GetObjectItem(data, "employId")->valuestring);

			//sprintf( (mr->ud.realName), "%s", cJSON_GetObjectItem(data, "realName")->valuestring);
			getGB2312(cJSON_GetObjectItem(data, "realName")->valuestring,mr->ud.realName );
		}
	}
	else if (strcmp(function, "getDeviceTask") == 0)
	{
		mr->function	=	TASKGET;

		printf("getDeviceTask\n");
		data	= cJSON_GetObjectItem(root, "data");
		if(mr->status == MQTT_OK_STATUS && data != NULL)
		{
			taskNum	= cJSON_GetArraySize( data );
			for(iCnt=0;iCnt<taskNum && iCnt < TASKARRSIZE;iCnt++)
			{
				subdata			= cJSON_GetArrayItem(data, iCnt);
				if(mr->td[iCnt] == NULL)
				{
					mr->td[iCnt]	= (struct TaskData*)malloc( sizeof(struct TaskData) );
				}
				memset(mr->td[iCnt]->taskId,0,64);
				memset(mr->td[iCnt]->norms,0,64);
				memset(mr->td[iCnt]->totalWeight,0,64);
				memset(mr->td[iCnt]->complishWeight,0,64);
				memset(mr->td[iCnt]->status,0,64);
				memset(mr->td[iCnt]->status1,0,64);
				memset(mr->td[iCnt]->wireSize,0,64);
				memset(mr->td[iCnt]->instructionOrder,0,64);

				sscanf( cJSON_GetObjectItem(root, "currentPages")->valuestring,"%d",&(mr->page));

				sprintf((mr->td[iCnt]->taskId),"%s", cJSON_GetObjectItem(subdata, "taskId")->valuestring);
				sprintf((mr->td[iCnt]->norms),"%s", cJSON_GetObjectItem(subdata, "norms")->valuestring);
				sprintf((mr->td[iCnt]->instructionOrder),"%s", cJSON_GetObjectItem(subdata, "instructionOrder")->valuestring);
				sprintf((mr->td[iCnt]->wireSize),"%s", cJSON_GetObjectItem(subdata, "wireSize")->valuestring);
				sprintf((mr->td[iCnt]->totalWeight),"%s", cJSON_GetObjectItem(subdata, "totalWeight")->valuestring);
				sprintf((mr->td[iCnt]->complishWeight),"%s", cJSON_GetObjectItem(subdata, "complishWeight")->valuestring);
				sprintf((mr->td[iCnt]->status1),"%s", cJSON_GetObjectItem(subdata, "status")->valuestring);

				getGB2312(cJSON_GetObjectItem(subdata, "status")->valuestring,mr->td[iCnt]->status );

				//for(iv = 0;iv<strlen(cJSON_GetObjectItem(subdata, "status")->valuestring)hh);
			}
			//if(subdata != NULL)	cJSON_Delete(subdata);
			mr->curTaskNum	= iCnt;
		}

		//if(data != NULL)	cJSON_Delete(data);

		printf("getDeviceTask end read task:%d and page:%d\n",taskNum,mr->page);

	}
	else if (strcmp(function, "taskStart") == 0)
	{
		mr->function	=	TASKSTART;
		sprintf(mr->taskId, "%s", cJSON_GetObjectItem(root, "taskId")->valuestring);
		if( mr->status == 1 )
		{
			/* find the vector id in mr->td */
			for(iv=0;iv<mr->curTaskNum;iv++)
			{
				if(strncmp(mr->taskId,mr->td[iv]->taskId,strlen(mr->taskId)) == 0)break;
			}


			if(iv < mr->curTaskNum)
			{
				/* ascii code */
				sprintf(mr->td[iv]->status1, "%s", cJSON_GetObjectItem(root, "data")->valuestring);
				/* gb2312 code */
				getGB2312(cJSON_GetObjectItem(root, "data")->valuestring,mr->td[iv]->status );
			}
		}
	}
	else if (strcmp(function, "taskEnd") == 0)
	{
		mr->function	=	TASKEND;
		sprintf(mr->taskId, "%s", cJSON_GetObjectItem(root, "taskId")->valuestring);

		if( mr->status == 1 )
		{
			/* find the vector id in mr->td */
			for(iv=0;iv<mr->curTaskNum;iv++)
			{
				if(strncmp(mr->taskId,mr->td[iv]->taskId,strlen(mr->taskId)) == 0)break;
			}

			if(iv < mr->curTaskNum)
			{
				/* ascii code */
				sprintf(mr->td[iv]->status1, "%s", cJSON_GetObjectItem(root, "data")->valuestring);
				/* gb2312 code */
				getGB2312(cJSON_GetObjectItem(root, "data")->valuestring,mr->td[iv]->status );
			}
		}

	}
	else if (strcmp(function, "taskPause") == 0)
	{
		mr->function	=	TASKPAUSE;
		sprintf(mr->taskId, "%s", cJSON_GetObjectItem(root, "taskId")->valuestring);

		if( mr->status == 1 )
		{
			/* find the vector id in mr->td */
			for(iv=0;iv<mr->curTaskNum;iv++)
			{
				if(strncmp(mr->taskId,mr->td[iv]->taskId,strlen(mr->taskId)) == 0)break;
			}

			if(iv < mr->curTaskNum)
			{
				/* ascii code */
				sprintf(mr->td[iv]->status1, "%s", cJSON_GetObjectItem(root, "data")->valuestring);
				/* gb2312 code */
				getGB2312(cJSON_GetObjectItem(root, "data")->valuestring,mr->td[iv]->status );
			}
		}
	}
	else if (strcmp(function, "alarm") == 0)
	{
		mr->function	=	TASKALARM;
		sprintf(mr->taskId, "%s", cJSON_GetObjectItem(root, "taskId")->valuestring);

		if( mr->status == 1 )
		{
			/* find the vector id in mr->td */
			for(iv=0;iv<mr->curTaskNum;iv++)
			{
				if(strncmp(mr->taskId,mr->td[iv]->taskId,strlen(mr->taskId)) == 0)break;
			}

			if(iv < mr->curTaskNum)
			{
				memset(mr->td[iv]->alarm,0,64);
				memset(mr->td[iv]->alarmdata,0,SIZE_PRINT_MSG);

				/* ascii code for data:yellow/red	*/
				sprintf(mr->td[iv]->alarm, "%s", cJSON_GetObjectItem(root, "data")->valuestring);
				/* gb2312 code for data:detail data */
				getGB2312(cJSON_GetObjectItem(root, "alarmdata")->valuestring,mr->td[iv]->alarmdata);
				printf("%s\n",mr->td[iv]->alarmdata);
			}
		}
	}
	else if (strcmp(function, "print") == 0)
	{
		mr->function	=	TASKPRINT;
		sprintf(mr->taskId, "%s", cJSON_GetObjectItem(root, "taskId")->valuestring);
		sprintf(mr->printData, "%s", cJSON_GetObjectItem(root, "data")->valuestring);
	}

	//print_succes_back(mosq,root);//是否回调，未知
	cJSON_Delete(root);

	return 0;
}
