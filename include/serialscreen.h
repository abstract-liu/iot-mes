/*
 * print.h
 *
 *  Created on: Oct 14, 2020
 *      Author: lj
 */
#ifndef INCLUDE_SERIALSCREEN_H_
#define INCLUDE_SERIALSCREEN_H_

#define		FRM_LEN		128


/* for assemble */
#define	POS_TPY	2	
#define	POS_SCN	3
#define	POS_CTL	5
#define	POS_CMD	7
#define	POS_RCD	7
#define	POS_LEN	9
#define	POS_UPDOWN	9
#define	POS_MUSICID	3
#define	POS_SEC		2

/* for resolve */
#define	POS_TYPE	7

/* extra 1byte type 0x11 or 0x1D*/
#define	POS_USER	8
#define	POS_PASS	8
#define	POS_OD_RCD	8
//EE B1 11 00 01 00 01 11 31 30 30 31 BA C5 00FF FC FF FF
typedef	struct ComFrame
{
	int		frmlen;
	char	data[FRM_LEN];
}ComFrame,*ComFrameP;


struct task_list_args{
	int		task_id;
	char	job_number[64];
	char	model[1024];
	char	task_request[1024];
	char 	task_status[64];
}task_list_args;

#ifdef  __cplusplus  
extern "C" {  
#endif  
/* two pthreads */
void th_serialscreen_send(void);//线程
void th_serialscreen_recv(void);//线程

/* function list */
void th_serialscreen_recv(void);
void th_serialscreen_send(void);
int scn_cmd_switch(int scnid);
int scn_cmd_setctl(int scnid,int ctlid,char*content,int contentlen);
int scn_cmd_visible(int scnid,int ctlid,int visible);
int scn_cmd_emptyrecord(int scnid,int ctlid);
int scn_cmd_setsinglerecord(int scnid,int ctlid,struct MQTTReturn*mr,int recordid);
int scn_cmd_setmultiplerecord(int scnid,int ctlid,struct MQTTReturn*mr);
int scn_cmd_settext(int scnid,int ctlid,struct MQTTReturn*mr);



int scn_cmd_settextp(int scnid,int ctlid,char *p);
int scn_cmd_buzzeron(int time);
int scn_cmd_opmusic(int musicid,int op);
int scn_cmd_flash(int scnid,int ctlid,int cycle);
int scn_cmd_seticon(int scnid,int ctlid,int iconid);

#ifdef  __cplusplus  
}  
#endif
#endif /* INCLUDE_STATUS_H_ */
