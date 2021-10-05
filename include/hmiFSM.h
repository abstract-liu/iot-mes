#ifndef	__HMIFSM__
#define	__HMIFSM__

/*Widget id for GLOBAL */
#define	JUMP_WDG_EXIT			45

/*main	state */
/*stand for the screen id*/
#define	STATE_RFID			1
#define	STATE_USER			10
#define	STATE_MAIN			0
#define	STATE_DETAIL		4
#define	STATE_FACE			16

/*stand for face id*/
#define	     WDG_FACE		5
/*stand for the control id*/
/*Widget id for rfid login */
/*Widget id for rfid login jump */
#define	JUMP_WDG_USER		4
/*Widget id for user login jump */
#define	JUMP_WDG_USRLG		12
#define	JUMP_WDG_RETURN		44
#define	     WDG_USER		10
#define	     WDG_PASS		11
/*Widget id for record number */
#define	WDG_OD_RECORD		7
/*Widget id for main page jump */
#define	JUMP_WDG_OD_IMPORT	15
#define	JUMP_WDG_OD_DETAIL	30
#define	JUMP_WDG_OD_START	40
#define	JUMP_WDG_OD_PAUSE	41
#define	JUMP_WDG_OD_END		42
#define	JUMP_WDG_OD_PRINT	43

#define	JUMP_WDG_OD_PREV	36
#define	JUMP_WDG_OD_NEXT	39

#define	     WDG_TX_USER	4
/*Widget id for detail page */
/*Widget id for detail page jump */
#define	JUMP_WDG_DT_RETURN		44
#define	JUMP_WDG_DT_AUDIO_ON	46
#define	JUMP_WDG_DT_AUDIO_OFF	47
#define	     WDG_DT_NUM			51
#define	     WDG_DT_OD			52
#define	     WDG_DT_OD_NORMS	53	
#define	     WDG_DT_OD_RQM		54
#define	     WDG_DT_OD_STATUS	55
#define		 WDG_DT_END			42
#define		 WDG_DT_END1		22
#define		 WDG_DT_ALARMRED	7
#define		 WDG_DT_ALARMYELLOW	9	

/*Intermediate state */
#define	STATE_ORDER		6
#define	STATE_START		7
#define	STATE_PAUSE		8
#define	STATE_END		9
#define	STATE_PRINT		10
//size definitionn
#define	SIZE_USER		64
#define	SIZE_PASS		64
#define	SIZE_RFID		64
#define	SIZE_MQTT_MSG	8192
#define	SIZE_PAYLOAD	8192

#define	SIZE_PRINT_MSG	8192

#define	SIZE_TEMP		512
//login definitionn
#define	LOGIN			1
#define	NOTLOGIN		0
//record definitionn
#define	RDEMPTY			-1
//Task Data structure definitionn
#define	TASKARRSIZE		10

#define	PAGE_START		1
//function definitionn for FSM
#define	USRLOGIN		0	
#define	RFIDLOGIN		1
#define	TASKGET			2
#define	TASKSTART		3
#define	TASKPAUSE		4

#define	TASKGOON		5
#define	TASKEND			6
#define	TASKPRINT		7
#define	TASKPREV		8
#define	TASKNEXT		9
#define	TASKDETAIL		10
#define	FACELOGIN		11
#define	TASKALARM		100

/*main state*/
#define	STAT_NOTPROD	2
#define	STAT_PAUSE		1	
#define	STAT_PRODUCE	0
#define	STAT_END		3
/*icon*/
#define	ICON_NOTPROD	0
#define	ICON_PAUSE		2
#define	ICON_PRODUCE	1
#define	ICON_END		3
/*alarm*/
#define	ALARM_RED		0
#define	ALARM_YELLOW	1

typedef	struct CurState
{
	int		screenid;
	int		widgetid;
	int		widgetJump;
	/*for user pass login*/
	char	user[SIZE_USER];
	char	pass[SIZE_PASS];
	/*for rfid login*/
	char	rfid[SIZE_RFID];
	/*for login*/
	int		login;
	/*for order import*/
	/*for order start/end/pause/print */
	int		recordid;
	int		orderid;
}CurState,*CurStateP;

typedef struct UserData
{
	char	userAccount[64];
	char	realName[64];
	char	employId[64];
}UserData,*UserDataP;
typedef struct TaskData
{
	char	clientId[64];
	int		complishLenght;
	//float	complishWeight;
	char	complishWeight[64];
	int		createTime;
	char	deviceId[64];
	char	id[64];
	int		initialLenght;
	int		instructionDate;
	char	instructionOrder[64];
	int		lastPrintLength;
	int		lastPrintTime;
	float	lastPrintWeight;
	char	norms[64];
	float	pauseWeight;
	float	preHour;
	char	productId[64];
	char	productName[64];
	float	speed;
	int		startTime;
	char	status[64];
	char	status1[64];
	char	taskId[64];
	char	totalWeight[64];
	//float	totalWeight;
	char	unit[64];
	int		updateTime;
	char	wireSize[64];
	/* alarm status */
	char	alarm[64];
	char	alarmdata[SIZE_PRINT_MSG];
	struct  TaskData* next;
}TaskData,*TaskDataP;
typedef	struct MQTTReturn
{
	/* describe mqtt request session*/
	int		status;
	int		function;
	char	gatewayId[64];
	char	sequence[16];
	char	taskId[64];
	/* describe print data*/
	char	printData[SIZE_PRINT_MSG ];
	/* describe user login*/
	int		page;
	/* describe user login*/
	int		login ;
	struct	UserData ud;
	char	rfid[SIZE_RFID];
	/* describe tasks */
	struct	TaskData* td[TASKARRSIZE];
	int		curTaskNum;
}MQTTReturn,*MQTTReturnP;

#ifdef  __cplusplus  
extern "C" {  
#endif  


/* utils functions */
int getScreenID(char *cmd);
int getWidgetID(char *cmd);
int getWidgetState(char *cmd);

/* main thread for hmiFSM */
void *th_hmiFSM();


#ifdef  __cplusplus  
}  
#endif


#endif
