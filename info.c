#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#include "mosquitto.h"
#include "sqlite3.h"

#include "global.h"
#include "hmiFSM.h"
#include "info.h"
#include "mcuio.h"

extern pthread_mutex_t db_mtx;

/* 数据库路径 */
extern char database_path[128];

/* MQTT信息 */
extern char mqtt_host[50];
extern int mqtt_port;

extern int gateway_id;
extern char factory_name[128];

static int mqtt_select_callback(void *notUsed, int argc, char **argv,
		char **azColName)
{
	strcpy(mqtt_host, argv[0]);
	mqtt_port = atoi(argv[1]);
	return 0;
}
static int factory_select_callback(void *notUsed, int argc, char **argv,
		char **azColName)
{
	strcpy(factory_name, argv[0]);
	return 0;
}
void get_dev_info(void)
{

	get_id();

}

void get_mqtt_info(void)
{
	int rc;
	char *zErrMsg = 0;
	sqlite3 *db;

	char sql_mqtt[128] =
			"SELECT serverAddr, serverPort FROM mqttInfo WHERE isUsed = 1;";

	pthread_mutex_lock(&db_mtx);
	rc = sqlite3_open(database_path, &db);
	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	rc = sqlite3_exec(db, sql_mqtt, mqtt_select_callback, NULL, &zErrMsg);
	sqlite3_close(db);
	pthread_mutex_unlock(&db_mtx);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		printf("MQTT Broker: %s\nPort: %d\n", mqtt_host, mqtt_port);
	}

	return;
}

void get_mqtt_topic(topic_send)
{
	strcpy(topic_send,"yuanhong/workshop1/mes");
}
void get_factory_info(void)
{
	int rc;
	char *zErrMsg = 0;
	sqlite3 *db;

	char sql_factory[128] = "SELECT factory_name FROM factoryInfo ;";

	pthread_mutex_lock(&db_mtx);
	rc = sqlite3_open(database_path, &db);
	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	rc = sqlite3_exec(db, sql_factory, factory_select_callback, NULL, &zErrMsg);
	sqlite3_close(db);
	pthread_mutex_unlock(&db_mtx);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		printf("factory_name : %s\n", factory_name);
	}

	return;
}


void get_id(void)
{
	int id;
	id=getID();
	printf("id==%d\n",id);
	gateway_id = id;
}


int getGB2312(char*ssrc,char*dst)
{
	printf("In getGB2312 function\n ");
	int ti1,ti2;
	char src[SIZE_MQTT_MSG];

	memset(src,0,SIZE_MQTT_MSG);
	memcpy(src,ssrc,SIZE_MQTT_MSG);
	int src_len		= strlen(ssrc);

	for(ti1 = 0; ti1 < src_len ; ti1++)
	{
		if(src[ti1]>='0'&&src[ti1]<='9')
		{
			src[ti1]	-=	'0';
		}
		else if(src[ti1]>='A'&&src[ti1]<='Z')
		{
			src[ti1]	=	src[ti1]-'A'+10;
		}
		else if(src[ti1]>='a'&&src[ti1]<='z')
		{
			src[ti1]	=	src[ti1]-'a'+10;
		}
		else
		{
			printf("输入中存在非指定字符\n");
		}
		printf("%x ",src[ti1]);
	}
	printf("\nafter translate:\n");
	for(ti2 = 0; ti2 < src_len; ti2=ti2+2)
	{
		char tik;

		tik			=	src[ti2]*16 + src[ti2+1];
		dst[ti2/2]	=	tik;

		printf("%x ",dst[ti2/2]);
	}

	printf("\n\n");
	return 0;
}

int get_mac(char *mac_str)
{
	struct ifreq ifreq;
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Get MAC socket error!\n");
		return -1;
	}
#ifdef _RELEASE
	strcpy(ifreq.ifr_name, "eth1");
#endif
#ifdef _DEBUG
	strcpy(ifreq.ifr_name, "ens33");
#endif

	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
	{
		fprintf(stderr, "Get MAC ioctl error!\n");
		return -1;
	}

	sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",
			(unsigned char) ifreq.ifr_hwaddr.sa_data[0],
			(unsigned char) ifreq.ifr_hwaddr.sa_data[1],
			(unsigned char) ifreq.ifr_hwaddr.sa_data[2],
			(unsigned char) ifreq.ifr_hwaddr.sa_data[3],
			(unsigned char) ifreq.ifr_hwaddr.sa_data[4],
			(unsigned char) ifreq.ifr_hwaddr.sa_data[5]);

	close(sock);

	return 0;
}

int get_time(char *time_str)
{
	time_t now;
	struct tm *local;
	time(&now);
	local = localtime(&now);
	sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d", local->tm_year + 1900,
			local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min,
			local->tm_sec);
	return 0;
}
