
#include <iostream>

using namespace std;

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "cQueue.h"
#include "myQueue.h"
#include "myMQTT.h"
#include "serialscreen.h"
#include "hmiFSM.h"
#include "cJSON.h"
#include "camera.h"
#include "sscreenupdate.h"
#include "base64.h"
#include "faceSearch.h"

char *statuslist[]={"C9FAB2FAD6D0","D2D1D4DDCDA3","CEB4C9FAB2FA","D2D1BDE1CAF8"};
char *alarmlist[]={"red","yellow"};

extern Queue 		* qrfid;		
extern Queue 		* qscreensend;
extern Queue 		* qscreenrecv;
extern CQueue 		* qmqttsend;
extern CQueue 		* qmqttrecv;
extern CQueue 		* qprintrecv;

extern pthread_mutex_t mutex_rfidqueue;		
extern pthread_mutex_t mutex_scnsendqueue;
extern pthread_mutex_t mutex_scnrecvqueue;
extern pthread_mutex_t mutex_mqttrecvqueue;
extern pthread_mutex_t mutex_mqttsendqueue;
extern pthread_mutex_t mutex_printrecvqueue;

int getScreenID(char *cmd);
int getUser(char *cmd,char *user);
int getPass(char *cmd,char *pass);
int getOrderRecordID(char *cmd);

#define SIZE_PIC		2*1024*1024	/*1M Bytes*/
#define SIZE_CMD		1*1024		/*1K Bytes*/
unsigned char	pic[SIZE_PIC];
unsigned char	base64[SIZE_PIC];
extern char mac[];
cJSON *rootface;
char temp[1024]={0};
char tempuser[1024]={0};

void FSM_init(struct MQTTReturn *mr,struct CurState cs[5] )
{
	memcpy(mr->ud.userAccount,"123456",6);
	memcpy(mr->ud.realName,"张三",4);
	memcpy(mr->ud.employId,"654321",6);

	memcpy(mr->rfid,"7593879C",8);

	mr->login	= NOTLOGIN;
	mr->page	= PAGE_START;

	cs[STATE_RFID].screenid		= STATE_RFID;
	cs[STATE_USER].screenid		= STATE_USER;
	cs[STATE_MAIN].screenid		= STATE_MAIN;
	cs[STATE_DETAIL].screenid	= STATE_DETAIL;

	cs[STATE_RFID].recordid		= RDEMPTY;
	cs[STATE_USER].recordid		= RDEMPTY;
	cs[STATE_MAIN].recordid		= RDEMPTY;
	cs[STATE_DETAIL].recordid	= RDEMPTY;

	cs[STATE_RFID].login= NOTLOGIN;
	cs[STATE_USER].login= NOTLOGIN;
	cs[STATE_MAIN].login= NOTLOGIN;
	cs[STATE_DETAIL].login= NOTLOGIN;

	memset(cs[STATE_USER].user,0,SIZE_USER);
	memset(cs[STATE_USER].pass,0,SIZE_PASS);
	memset(cs[STATE_RFID].rfid,0,SIZE_RFID);

}
void *th_hmiFSM()
{
	int scnid		=-1,wdgid=-1,btnup;
	int				iv=0,page;
	char			*cmd;
	char			msg[SIZE_MQTT_MSG];
	char			user[64];
	char			pass[64];
	char			dtnum[64];
	char			require[SIZE_TEMP];

	struct CurState cs[16];
	struct MQTTReturn mr;

	/* intermediate data */
	QNode	front;
	CQNode	cfront;
	struct	ComFrame cf;	
	Itemt1	itm;


	FSM_init(&mr,cs);

	/* screen switch to rfid login */
	scn_cmd_switch(STATE_RFID);
	//scn_cmd_switch(STATE_MAIN	);

	while(true)
	{
		/* State transition by screen command */
		/* handle the data from screen */
		if( !IsEmptyQueue(qscreenrecv))
		{
			front		= qscreenrecv->front;
			cf.frmlen	= front->data->frmlen;
			memcpy(cf.data,front->data->data,front->data->frmlen);

			pthread_mutex_lock(&mutex_scnrecvqueue);
			DeQueue(qscreenrecv);
			pthread_mutex_unlock(&mutex_scnrecvqueue);

			cmd		= cf.data;
			scnid	= getScreenID(cmd);
			wdgid	= getWidgetID(cmd);
			btnup	= getWidgetState(cmd);

			/* button up only */
			if( btnup == 1 )
				continue;

			printf("scnid:%d\twdgid:%d\n",scnid,wdgid);

			if(wdgid == JUMP_WDG_EXIT)
			{
				mr.login = NOTLOGIN;
				memset(cs[STATE_USER].user,0,SIZE_USER);
				memset(cs[STATE_USER].pass,0,SIZE_PASS);
				memset(cs[STATE_RFID].rfid,0,SIZE_RFID);

				scn_cmd_switch(STATE_RFID);
				scn_cmd_opmusic(1,1);
			}
			else if(cs[STATE_RFID].screenid == scnid && false)
			{
				//do nothing
			}
			//else if(cs[STATE_FACE].screenid == scnid)scnid == STATE_RFID && wdgid  == WDG_FACE 
			else if( (scnid == STATE_RFID) && (wdgid  == WDG_FACE) ) 
			{
				//do nothing
				rootface = cJSON_CreateObject();
				cJSON_AddItemToObject(rootface, "image_type", cJSON_CreateString("BASE64"));
				cJSON_AddItemToObject(rootface, "group_id_list", cJSON_CreateString("Dafeng,S204"));
				cJSON_AddItemToObject(rootface, "quality_control", cJSON_CreateString("NORMAL"));
				cJSON_AddItemToObject(rootface, "liveness_control", cJSON_CreateString("NORMAL"));

				memset(pic,0,SIZE_PIC);
				memset(base64,0,SIZE_PIC);

				int length = get_Image();
				//scn_cmd_setctl( 16,3,"Get Image success..",strlen("Get Image success..") );

				int reallen = base64_encode(pic+5, (unsigned int)length, (char*)base64);

				cJSON_AddItemToObject(rootface, "image", cJSON_CreateString((const char*)base64));

				memset( temp,0,1024);
				memcpy( temp,"Face Searching...",strlen("Face Searching..."));
				//scn_cmd_setctl( 16,3,temp,strlen(temp) );

				std::string retval;
				retval = faceAuth(rootface); 

				cJSON_Delete(rootface);

				cJSON * ret = cJSON_Parse( retval.data() );

				if ( !ret ) 
				{
					printf("Error before: [%s]\n",cJSON_GetErrorPtr());
					continue;
				}

				memset( temp,0,1024);
				memset( tempuser,0,1024);
				if( cJSON_GetObjectItem(ret,"error_code")->valueint != 0 )
				{
					strcat(temp,cJSON_GetObjectItem(ret, "error_msg")->valuestring );
					std::cout<<"error_code:"<<temp<<std::endl;
					//scn_cmd_setctl( 16,3,temp,strlen(temp) );
					scn_cmd_switch(STATE_RFID	);
				}
				else
				{
					cJSON * tmpj	= cJSON_GetObjectItem(ret, "result");
					cJSON * data	= cJSON_GetObjectItem(tmpj, "user_list");

					if( data != NULL )
					{
						cJSON * subdata		= data->child;

						strcat(temp,cJSON_GetObjectItem(subdata, "group_id")->valuestring );
						strcat(tempuser,cJSON_GetObjectItem(subdata, "user_id")->valuestring );
						//scn_cmd_setctl( 16,3,tempuser,strlen( tempuser ));

						mqtt_send_cmd(FACELOGIN,mac,tempuser,NULL,NULL,NULL,NULL,0);
						scn_cmd_switch(STATE_MAIN	);
						//sleep(3);
					}
				}

				cJSON_Delete(ret);
			}
			else if(cs[STATE_USER].screenid == scnid)
			{
				if( wdgid == JUMP_WDG_USRLG)
				{
					mqtt_send_cmd(USRLOGIN,mac,cs[STATE_USER].user,cs[STATE_USER].pass,NULL,NULL,NULL,0);
				}
				else if( wdgid == WDG_USER)
				{
					memset(user,0,SIZE_USER);
					getUser(cmd,user);
					printf("getUser:%s\n",user);
					memset(cs[STATE_USER].user,0,SIZE_USER);
					memcpy(cs[STATE_USER].user,user,strlen(user));
				}
				else if( wdgid == WDG_PASS)
				{
					memset(pass,0,SIZE_PASS);
					getPass(cmd,pass);

					memset(cs[STATE_USER].pass,0,SIZE_PASS);
					memcpy(cs[STATE_USER].pass,pass,strlen(pass));

					printf("getPass:%s\n",cs[STATE_USER].pass);
				}
			}
			else if(cs[STATE_MAIN].screenid == scnid)
			{
				if( wdgid == WDG_OD_RECORD )
				{
					cs[STATE_MAIN].recordid = getOrderRecordID(cmd);
				}
				else if( wdgid == JUMP_WDG_OD_IMPORT )
				{
					page = mr.page<1?1:mr.page;
					mqtt_send_cmd(TASKGET,mac,NULL,NULL,mr.ud.employId,NULL,NULL,page);
				}
				else if( wdgid== JUMP_WDG_OD_START)
				{
					//printf("OD_START recordid:%d\ttaskId:%s\n",cs[STATE_MAIN].recordid,mr.td[cs[STATE_MAIN].recordid]->taskId);
					if(cs[STATE_MAIN].recordid != RDEMPTY)
						mqtt_send_cmd(TASKSTART,mac,NULL,NULL,NULL,NULL,mr.td[cs[STATE_MAIN].recordid]->taskId,0);
				}
				else if( wdgid == JUMP_WDG_OD_PAUSE)
				{
					if(cs[STATE_MAIN].recordid != RDEMPTY)
						mqtt_send_cmd(TASKPAUSE,mac,NULL,NULL,NULL,NULL,mr.td[cs[STATE_MAIN].recordid]->taskId,0);
				}
				else if( wdgid == JUMP_WDG_OD_END)
				{
					//if(cs[STATE_MAIN].recordid != RDEMPTY)
					//	mqtt_send_cmd(TASKEND,mac,NULL,NULL,NULL,NULL,mr.td[cs[STATE_MAIN].recordid]->taskId,0);
					/* end invisble*/

					if(cs[STATE_MAIN].recordid != RDEMPTY)
					{
						/* end invisble*/
						scn_cmd_visible(STATE_DETAIL,WDG_DT_END,1);
						scn_cmd_visible(STATE_DETAIL,WDG_DT_END1,1);
						/* num */
						memset(dtnum,0,64);
						sprintf( dtnum,"%d",cs[STATE_MAIN].recordid+1);
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_NUM,		dtnum );

						/* taskid, norms, totalWeight */
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD,		mr.td[cs[STATE_MAIN].recordid]->taskId );
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD_NORMS,	mr.td[cs[STATE_MAIN].recordid]->norms );

						/* detail requirements */
						memset(require,0,SIZE_TEMP);
						//sprintf( require,"%s/%s","Requirements:",mr.td[cs[STATE_MAIN].recordid]->totalWeight);
						//sprintf( require,"%s%s/%s/%s","Requirements:",mr.td[iv]->complishWeight,mr.td[iv]->totalWeight);
						sprintf( require,"%s%s/%s/%s","Requirements:",mr.td[cs[STATE_MAIN].recordid]->wireSize,mr.td[cs[STATE_MAIN].recordid]->complishWeight,mr.td[cs[STATE_MAIN].recordid]->totalWeight);
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD_RQM, require );

						/* icon */
						if(strcmp(statuslist[STAT_NOTPROD],mr.td[cs[STATE_MAIN].recordid]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_NOTPROD );
						}
						else if( strcmp( statuslist[STAT_PAUSE],mr.td[cs[STATE_MAIN].recordid]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_PAUSE );
						}
						else if( strcmp(statuslist[STAT_PRODUCE],mr.td[cs[STATE_MAIN].recordid]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_PRODUCE );
						}
						else
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_END);

						/* screen switch */
						scn_cmd_switch(STATE_DETAIL		);
					}
				}
				else if( wdgid == JUMP_WDG_OD_PRINT)
				{
					if(cs[STATE_MAIN].recordid != RDEMPTY)
						mqtt_send_cmd(TASKPRINT,mac,NULL,NULL,NULL,NULL,mr.td[cs[STATE_MAIN].recordid]->taskId,0);
				}
				/* front end control without server interaction */
				else if( wdgid == JUMP_WDG_OD_PREV	)
				{
					page = mr.page-1<1?1:mr.page-1;
					mr.page	= mr.page - 1;
					mqtt_send_cmd(TASKGET,mac,NULL,NULL,mr.ud.employId,NULL,NULL,page);
				}
				else if( wdgid == JUMP_WDG_OD_NEXT )
				{
					page = mr.page+1;
					mqtt_send_cmd(TASKGET,mac,NULL,NULL,mr.ud.employId,NULL,NULL,page);
				}
				else if( wdgid == JUMP_WDG_OD_DETAIL)
				{
					if(cs[STATE_MAIN].recordid != RDEMPTY)
					{
						/* alarm invisble*/
						scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMYELLOW,0);
						scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMRED   ,0);

						/* end invisble*/
						scn_cmd_visible(STATE_DETAIL,WDG_DT_END,0);
						scn_cmd_visible(STATE_DETAIL,WDG_DT_END1,0);

						/* alarm */
						iv= cs[STATE_MAIN].recordid;
						if(strncmp(mr.td[iv]->alarm,alarmlist[ALARM_RED],strlen(alarmlist[ALARM_RED])) == 0)
						{  
							/* flash red */
							scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMYELLOW, 0);
							scn_cmd_visible(STATE_DETAIL, WDG_DT_ALARMRED   ,1);
							scn_cmd_settextp(STATE_DETAIL,WDG_DT_ALARMRED,  mr.td[iv]->alarmdata );
							scn_cmd_flash(STATE_DETAIL,WDG_DT_ALARMRED   ,10);
							scn_cmd_buzzeron(150);

							scn_cmd_visible(STATE_DETAIL,WDG_DT_END,1);
							scn_cmd_visible(STATE_DETAIL,WDG_DT_END1,1);
						}

						if(strncmp(mr.td[iv]->alarm,alarmlist[ALARM_YELLOW],strlen(alarmlist[ALARM_YELLOW])) == 0)
						{
							/* flash yellow */
							scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMYELLOW,1);
							scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMRED   ,0);
							scn_cmd_settextp(STATE_DETAIL,WDG_DT_ALARMYELLOW,mr.td[iv]->alarmdata );
							scn_cmd_flash(STATE_DETAIL,   WDG_DT_ALARMYELLOW,10);
							scn_cmd_buzzeron(150);

							scn_cmd_visible(STATE_DETAIL,WDG_DT_END,1);
							scn_cmd_visible(STATE_DETAIL,WDG_DT_END1,1);
						}


						/* set user realName */
						scn_cmd_settext(STATE_DETAIL,WDG_TX_USER	,&mr);

						/* num */
						memset(dtnum,0,64);
						sprintf( dtnum,"%d",cs[STATE_MAIN].recordid+1);
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_NUM,		dtnum );
						//scn_cmd_settextp(STATE_DETAIL,WDG_DT_NUM,	mr.ud.realName	);

						/* taskid, norms, totalWeight */
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD,		mr.td[cs[STATE_MAIN].recordid]->taskId );
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD_NORMS,	mr.td[cs[STATE_MAIN].recordid]->norms );

						/* detail requirements */
						memset(require,0,SIZE_TEMP);
						sprintf( require,"%s%s/%s/%s","Requirements:",mr.td[cs[STATE_MAIN].recordid]->wireSize,mr.td[cs[STATE_MAIN].recordid]->complishWeight,mr.td[cs[STATE_MAIN].recordid]->totalWeight);
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD_RQM, require );

						/* icon */
						if(strcmp(statuslist[STAT_NOTPROD],mr.td[cs[STATE_MAIN].recordid]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_NOTPROD );
						}
						else if( strcmp( statuslist[STAT_PAUSE],mr.td[cs[STATE_MAIN].recordid]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_PAUSE );
						}
						else if( strcmp(statuslist[STAT_PRODUCE],mr.td[cs[STATE_MAIN].recordid]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_PRODUCE );
						}
						else
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, STAT_END);

						/* screen switch */
						scn_cmd_switch(STATE_DETAIL		);

					}
				}
			}
			else if(cs[STATE_DETAIL].screenid == scnid)
			{
				if( wdgid == JUMP_WDG_DT_RETURN )
				{
					scn_cmd_switch(STATE_MAIN	);
				}
				else if( wdgid == WDG_DT_END)
				{
					printf("recordid:%d\n",cs[STATE_MAIN].recordid );
					if(cs[STATE_MAIN].recordid != RDEMPTY)
					{
						mqtt_send_cmd(TASKEND,mac,NULL,NULL,NULL,NULL,mr.td[cs[STATE_MAIN].recordid]->taskId,0);
					}

					//cs[STATE_MAIN].recordid = getOrderRecordID(cmd);
					scn_cmd_switch(STATE_MAIN	);
				}
			}
		}

		/* handle the data from rfid */
		if( !IsEmptyQueue(qrfid))
		{
			printf("rfid\n");
			front		= qrfid->front;
			cf.frmlen	= front->data->frmlen;
			memcpy(cf.data,front->data->data,front->data->frmlen);

			pthread_mutex_lock(&mutex_rfidqueue);
			DeQueue(qrfid);
			pthread_mutex_unlock(&mutex_rfidqueue);

			memset(mr.rfid,0,SIZE_RFID);
			memcpy(mr.rfid,cf.data,cf.frmlen);

			if(mr.login == LOGIN)
			{
				/* do nothing if login */
				//scn_cmd_switch(STATE_MAIN	);

				/* avoid the situation of screen close depently */
				//scn_cmd_settext(STATE_MAIN,WDG_TX_USER	,&mr);
				//mqtt_send_cmd(TASKGET,mac,NULL,NULL,mr.ud.employId,NULL,NULL,page);
				mqtt_send_cmd(RFIDLOGIN,mac,NULL,NULL,NULL,mr.rfid,NULL,0);
			}
			else if(mr.login == NOTLOGIN)
			{
				mqtt_send_cmd(RFIDLOGIN,mac,NULL,NULL,NULL,mr.rfid,NULL,0);
			}
		}

		/* handle the data from mqtt */
		if( !IsEmptyCQueue(qmqttrecv))
		{
			memset(msg,0,SIZE_MQTT_MSG);

			cfront = qmqttrecv->front;
			memcpy(msg,cfront->data,strlen(cfront->data));

			pthread_mutex_lock(&mutex_mqttrecvqueue);
			DeCQueue(qmqttrecv);
			pthread_mutex_unlock(&mutex_mqttrecvqueue);

			pthread_mutex_lock(&mutex_mqttrecvqueue);
			get_jsondata(&mr,msg);
			pthread_mutex_unlock(&mutex_mqttrecvqueue);

			switch( mr.function )
			{
				case USRLOGIN:
					if(mr.status == 0)
					{
						/* login failed*/
						mr.login = NOTLOGIN;
					}
					else if( mr.login == NOTLOGIN )
					{
						/* login success */
						mr.login = LOGIN;
						scn_cmd_settext(STATE_MAIN,WDG_TX_USER	,&mr);
						scn_cmd_switch(STATE_MAIN	);
					}
					break;
				case FACELOGIN:
					if(mr.status == 0)
					{
						mr.login = NOTLOGIN;
					}
					else if( mr.login == NOTLOGIN )
					{
						/* login success */
						mr.login = LOGIN;
						scn_cmd_settext(STATE_MAIN,WDG_TX_USER	,&mr);
						scn_cmd_switch(STATE_MAIN	);
					}
					break;
				case RFIDLOGIN:
					if(mr.status == 0)
					{
						/* login failed*/
						mr.login = NOTLOGIN;
					}
					else if( cs[STATE_USER].login == NOTLOGIN )
					{
						/* login success */
						mr.login = LOGIN;
						scn_cmd_switch(STATE_MAIN	);
						scn_cmd_settext(STATE_MAIN,WDG_TX_USER	,&mr);
						/* mr.rfid store the temporary data*/
						/* cs[STATE_USER] store the right data*/
						memset(cs[STATE_USER].rfid,0,SIZE_RFID);
						memcpy(cs[STATE_USER].rfid,mr.rfid,SIZE_RFID);
						//memcpy(mr.rfid,mr.rfid,SIZE_RFID);
						scn_cmd_opmusic(0,1);

						/* import order automatically */
						mqtt_send_cmd(TASKGET,mac,NULL,NULL,mr.ud.employId,NULL,NULL,page);
					}
					else if( cs[STATE_USER].login == LOGIN )
					{
						/* avoid switch screen depently*/
						scn_cmd_switch( STATE_MAIN	);


					}
					break;
				/*for import/start/end/pause/print*/ 
				case TASKGET:
					if(mr.status == 1)
					{
						/* empty the record */ 
						scn_cmd_emptyrecord(STATE_MAIN, WDG_OD_RECORD);
						/* reset all the record */ 
						scn_cmd_setmultiplerecord(STATE_MAIN,WDG_OD_RECORD,&mr);
					}
					break;
				case TASKSTART:
					if(mr.status == 1)
					{
						/*way 1*/
						//scn_cmd_setsinglerecord(STATE_MAIN,WDG_OD_RECORD,&mr,cs[STATE_MAIN].recordid);
						/*way 2 ,better than way 1*/
						for(iv=0;iv<mr.curTaskNum;iv++)
						{
							if( strncmp(mr.taskId,mr.td[iv]->taskId,strlen(mr.taskId)) == 0)break;
						}

						printf("taskStart record num %d\t%s\t%s\t%s\n",iv,mr.td[iv]->taskId,mr.taskId,mr.td[iv]->status);
						if(iv < mr.curTaskNum)
							scn_cmd_setsinglerecord(STATE_MAIN,WDG_OD_RECORD,&mr,iv);
					}
					break;
				case TASKEND:
					if(mr.status == 1)
					{
						scn_cmd_setsinglerecord(STATE_MAIN,WDG_OD_RECORD,&mr,cs[STATE_MAIN].recordid);
					}
					break;
				case TASKPAUSE:
					if(mr.status == 1)
					{
						scn_cmd_setsinglerecord(STATE_MAIN,WDG_OD_RECORD,&mr,cs[STATE_MAIN].recordid);
					}
					break;
				case TASKALARM:
					if( (mr.status == 1) && (mr.login == LOGIN) )
					{
						for(iv=0;iv < mr.curTaskNum;iv++)
						{
							if(strncmp(mr.td[iv]->alarm,alarmlist[ALARM_RED],strlen(alarmlist[ALARM_RED])) == 0)break;
						}
						/* flash red */
						if(iv < mr.curTaskNum)
						{
							printf("alarmdata RED:%s\n",mr.td[iv]->alarmdata );
							scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMYELLOW,0);
							scn_cmd_visible(STATE_DETAIL, WDG_DT_ALARMRED   ,1);
							scn_cmd_settextp(STATE_DETAIL,WDG_DT_ALARMRED,  mr.td[iv]->alarmdata );
							scn_cmd_flash(STATE_DETAIL,WDG_DT_ALARMRED   ,10);
							scn_cmd_buzzeron(150);
						}

						if(iv >= mr.curTaskNum)
						{
							for(iv=0;iv < mr.curTaskNum;iv++)
							{
								if(strncmp(mr.td[iv]->alarm,alarmlist[ALARM_YELLOW],strlen(alarmlist[ALARM_YELLOW])) == 0)break;
							}
							/* flash yellow */
							if(iv < mr.curTaskNum)
							{
								scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMYELLOW,1);
								scn_cmd_visible(STATE_DETAIL,WDG_DT_ALARMRED   ,0);
								scn_cmd_settextp(STATE_DETAIL,WDG_DT_ALARMYELLOW,mr.td[iv]->alarmdata );
								scn_cmd_flash(STATE_DETAIL,   WDG_DT_ALARMYELLOW,10);
								scn_cmd_buzzeron(150);

								if(mr.td[iv]->alarmdata != NULL)
									printf("alarmdata Yellow:%s\n", "No Data");
								else
									printf("alarmdata Yellow:%s\n",mr.td[iv]->alarmdata );

							}
						}

						if(iv >= mr.curTaskNum)continue;

						/* end invisble*/
						scn_cmd_visible(STATE_DETAIL,WDG_DT_END,1);
						scn_cmd_visible(STATE_DETAIL,WDG_DT_END1,1);
						/* num */
						memset(dtnum,0,64);
						sprintf( dtnum,"%d",iv+1);
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_NUM,dtnum );

						/* taskid, norms, totalWeight */
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD,		mr.td[iv]->taskId );
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD_NORMS,	mr.td[iv]->norms );

						/* detail requirements */
						memset(require,0,SIZE_TEMP);
						//sprintf( require,"%s%s/%s","Requirements:",mr.td[iv]->complishWeight,mr.td[iv]->totalWeight);
						sprintf( require,"%s%s/%s/%s","Requirements:",mr.td[cs[STATE_MAIN].recordid]->wireSize,mr.td[cs[STATE_MAIN].recordid]->complishWeight,mr.td[cs[STATE_MAIN].recordid]->totalWeight);
						scn_cmd_settextp(STATE_DETAIL,WDG_DT_OD_RQM, require );


						/* icon */
						if(strcmp(statuslist[STAT_NOTPROD],mr.td[iv]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_NOTPROD );
						}
						else if( strcmp( statuslist[STAT_PAUSE],mr.td[iv]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_PAUSE );
						}
						else if( strcmp(statuslist[STAT_PRODUCE],mr.td[iv]->status1) == 0)
						{
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, ICON_PRODUCE );
						}
						else
							scn_cmd_seticon(STATE_DETAIL,WDG_DT_OD_STATUS, 0 );

						/* set user name */
						scn_cmd_settext(STATE_DETAIL, WDG_TX_USER	,&mr);
						/* screen switch */
						scn_cmd_switch(STATE_DETAIL		);
					}
					break;
				case TASKPRINT:
					if( (mr.status == 1) && (mr.login == LOGIN) )
					{
						//scncmd pause
						memset( msg,0,SIZE_MQTT_MSG	);
						memcpy( msg,mr.printData,strlen(mr.printData) );
						itm		= msg;

						pthread_mutex_lock(&mutex_printrecvqueue);
						EnCQueue(qprintrecv,itm);
						pthread_mutex_unlock(&mutex_printrecvqueue);
					}
					break;
				default:
					break;
			}
		}

		//avoid too much cpu occupation;
		usleep(10*1000);
	}// end while
}

/* util functions for screen command data */
int getScreenID(char *cmd)
{
	int scnid;
	
	scnid = (cmd[POS_SCN]<<8) + cmd[POS_SCN + 1];
	return scnid;
}
int getWidgetID(char *cmd)
{
	int wdgid;
	
	wdgid = (cmd[POS_CTL]<<8) + cmd[POS_CTL+ 1];
	return wdgid;
}
int getWidgetState(char *cmd)
{
	int updown = 0;
	

	if(cmd[POS_TPY] == 0x11)
	{
		updown = cmd[POS_UPDOWN];
	}

	return updown;
}
int getOrderRecordID(char *cmd)
{
	int rcdid;
	
	rcdid = (cmd[POS_OD_RCD]<<8) + cmd[POS_OD_RCD+ 1];

	return rcdid;
}
int getUser(char *cmd,char *user)
{
	int		iv = POS_USER;
	
	for(iv=POS_USER;cmd[iv] != 0x00 && iv<64;iv++);

	if(iv>=64)return -1;

	memcpy(user,cmd+POS_USER,iv-POS_USER );
	return 0;
}
int getPass(char *cmd,char *pass)
{
	int		iv = POS_PASS;

	for(iv=0; cmd[iv] != 0xFC;iv++)printf(" %x",cmd[iv]);

	printf("getpass end\n");
	
	for(iv=POS_PASS;cmd[iv] != 0x00 && iv<64;iv++);

	if(iv>=64)return -1;

	memcpy(pass,cmd+POS_PASS,iv-POS_PASS);
	return 0;
}
