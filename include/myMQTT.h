#ifndef INCLUDE_MYMQTT_H_
#define INCLUDE_MYMQTT_H_


#define	MQTT_OK_STATUS	1

extern char mac[];

char topic_send[256];
char topic_serialscreen[256];



/* two pthreads */
void th_mqtt_recv(void);
void th_mqtt_send(void);

#ifdef  __cplusplus  
extern "C" {  
#endif  
/* two function for recv and send */
int	get_jsondata(struct MQTTReturn *mr,char*msg);
void mqtt_send_cmd(int function,char*gatewayid,char*user,char*pass,char*employid,char rfid[],char *taskid,int page );

#ifdef  __cplusplus  
}  
#endif
/* internal funcionts */
struct mosquitto* mqtt_init(void *message_callback, void *connect_callback);
int mqtt_quit(struct mosquitto *mosq);

int mqtt_pub(struct mosquitto *mosq, const char *topic, const char *msg);
void mqtt_sub(struct mosquitto *mosq, const char *topic);
void mqtt_sub_multiple(struct mosquitto *mosq, int topic_count,
		char *const*const topics);

#endif /* INCLUDE_MQTT_H_ */
