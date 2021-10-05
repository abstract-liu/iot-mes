#ifndef MYTHREAD_H_
#define MYTHREAD_H_
#include <sqlite3.h>


#define MAXLINE			128
//#define UART485DIR		"/dev/ttySP2"  
//#define UART232DIR		"/dev/ttySP0"

#define UART_PTR_DIR	"/dev/ttySP0"
#define UART_SCN_DIR	"/dev/ttySP1"  
//#define UART_RFID_DIR	"/dev/ttySP2"  
#define UART_CAMERA_DIR	"/dev/ttySP2"  

#define DEBUG			1
#define DEBUG_DEV 		1

#define DBDIR			"/opt/smartlib.db"
#define UPGRADEDIR		"/opt/version/smartlib"
//#define DBDIR_DEV 	    "/mnt/library-mqtt-noautoac/smartlib.db"
#define DBDIR_DEV		"/opt/smartlib.db"
#define DBDIR_RUN 	    "/opt/smartlib.db"
//#define SERVER_IP		"60.12.220.16"
//#define SERVER_IP		"218.75.15.78"
//#define SERVER_PORT				8322 
#define	AC_CLOSE_MIN_CURRENT	0.03

#define	TIMER_INTERVAL_LAMP		8
#define	TIMER_INTERVAL_IR		4
#define	TIMER_INTERVAL_DLY		1

#define	LIGHT_NUM				8


typedef struct server_config{
	char  mqtt_host1[128];
	int   mqtt_port1;
	char  path_main1[128];
	char  path_main_dev1[128];
	char  uartdir1[128];
	char  uart485dir1[128];
	char  pub_topic1[128];
	int   start_time;
	int   end_time;
}server_config;



typedef unsigned int uint;
typedef struct senddstr
{
	unsigned char head[4];
	unsigned char data[100];
	unsigned char tail[2];
}sendstr;

//recv buff is limits on zigbee
#define		MAX_DATA_LEN	40
//parameter
#define		TEMP_FLOAT	5
#define		HUM_FLOAT	5
#define		TEMP_OFFSET	5
#define		HUM_OFFSET	6

//Command_TYPE   app发给网关的命令类型
#define     EXCUTE									0x01          		//执行命令
#define	    GET										0x03				//获取信息命令

//TYPE		网关接受和发送命令类型
#define 	TYPE_G_RUN            					0x11                //网关向终端发送执行命令
#define 	TYPE_G_RUN_INFO	 						0x14			    //命令执行情况
#define 	TYPE_G_ASK_INFO  						0x12            	//网关向终端发送请求信息命令
#define 	TYPE_G_ASK_INFO_ACT           			0x10				//终端向网关返回请求命令消息

//SUB_TYPE	设备类型
#define 	SUBTYPE_SWITCH   						0x11   				//开关
#define 	SUBTYPE_CURTAIN 						0x14          		//窗帘
#define 	SUBTYPE_SOCKET      					0x19         		//插座
#define 	SUBTYPE_TEMPERATURE   					0x22    			//温度
#define 	SUBTYPE_HUMIDITY   						0x23				//湿度
#define 	SUBTYPE_TEMP_HUM		 				0x24          		//温湿度
#define 	SUBTYPE_LIGHT_INTENSITY      			0x27				//光照强度
#define  	SUBTYPE_AC                              0x43
#define     SUBTYPE_SECURITY						0x30				//安防
#define 	SUBTYPE_INFRARED_CONTROL 	      		0x40   				//红外控制
//add by zqf 20200104
#define 	SUBTYPE_INFRARED_LEARN	 	      		0xb1   				//红外学习
#define 	SUBTYPE_INFRARED_CLEAR_IR_FLASH			0xb2   				//红外学习清除所有
#define 	SUBTYPE_INFRARED_CLEAR_IR_ONE			0xb3   				//红外学习清除一条

#define 	SUBTYPE_AUTO_DOOR						0x31 				//智能门
//#define 	SUBTYPE_AC_SWITCH						0x32 				//ac switch
#define 	SUBTYPE_BODY_SENSOR						0x32 				//人体感应

// Message Header index 
#define   	MESSAGE_TYPE				0x7              	//获取状态值
#define   	MESSAGE_LEN					0x8              	//获取状态值
//ACTION   功能
#define   	FUNC_INFO				0x30              	//获取状态值
#define   	ILLUMINATION        	0x2b			  	//获取光照强度值
#define   	TEM_HUM        			0x2d              	//获取温湿度
#define 	BODY_SENSOR				0x2a				//获取人体感应的值
#define   	ILLUMINATION_SLEEP 		0x2c              	//临时定的协议就是看光感隔多少秒发送
#define   	FUNC_INFO				0x30              	//获取状态值
#define  	CRON_VIEW   			0xfe              	//查看定时任务
#define 	CRON_CANCEL				0xfd			  	//取消定时任务
#define  	OPEN					0x20			 	//打开
#define		CLOSE					0x21			 	//关闭
#define		INFRARED_CLOSE			0x0					//Infrared close
#define		INFRARED_OPEN			0x1					//Infrared open

int SystemStart();

void * socketConnect();
void * Sqlite3Thr();
// control and inquiry thread(UartCTLINQThr)
void * UartTXThr(void *str);

void * UartRXThr(void *command);
void * SendDataThr();

//数据库初始化
//sqlite3 * sqliteInit(char * name);
//内存数据库导入导出
//int loadOrSaveDb(sqlite3 *pInMemeory, const char *zFilename, int isSave);
int uartInit(char * name);
//开关命令
sendstr SwitchCommand(int phy_addrid,int commandid,int  action,int route,int command_type);

sendstr SecurityCommand(int phy_addrid,int commandid,int  action,int value,int command_type);
sendstr InfraredCommand(int phy_addrid,int commandid,int action);
int SECURITY(int phy_addr,int type,int value);

//void gatewayRegister();
int gatewayRegister();
//void * getInfFromDev(sendstr senddata,int connfd);
//void *WRSocketThr();

int control_Temp_Hum(int phy_addr_did,int value);
int control_Illumination(int phy_addr_did,int value);

void * RequestCommandSend();
sendstr commandRequest(int phy_addr,int action,int type);
void commmadSendThr(int subtype,int action);

void * judgeNobodyAndHandleThr();
void * BodySensorRequestThr();
void * RequestTem_HumSend();
void * RequestLightSend();


void * judgeBodySensorThr();
void * judgeTem_HumThr();
void * judgeIrLearnThr();
//判断光照强度值的大小
void * judgeIntensity();
int haveBody();


int control_DevCommandSend(int phy_addr_did,int route,int value);

void commmadSendBodyThr();
void commmadSendTemThr();
void commmadSendLightThr();

/*
void * judgeBodySensorThr(unsigned char * array);
void * judgeIntensity(unsigned char * array);
void * judgeTem_HumThr(unsigned char * array);
*/

//void * Sqlite3Thr(char *sqlstr);
void * Sqlite3Thr(void *sqlstr);
/**
 * response app control and status inquiry command
 */
void * UartTXThr(void *command);
#endif
