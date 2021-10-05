#include <pthread.h>

#include "global.h"

/* 数据库锁 */
pthread_mutex_t db_mtx;
/* 日志文件锁 */
pthread_mutex_t log_mtx;

/* 数据库路径 */
char database_path[128] = "/opt/dc/database.db";


/*公司名*/
char factory_name[128];
/* MQTT信息 */
char mqtt_host[50];
int mqtt_port;

/* 设备信息 */
int gateway_id;
char mac_addr[20];


int UART232ID;
int UART485ID;

int	UART_PTR_ID;
int	UART_SCN_ID;
int	UART_RFID_ID;
int	UART_CAMERA_ID;
