#ifndef INCLUDE_INFO_H_
#define INCLUDE_INFO_H_

void get_dev_info(void);
void get_mqtt_info(void);
void get_factory_info(void);


//void get_mqtt_topic(topic_send);


void get_id(void);
void get_factory_name(void);
int get_mac(char *mac_str);
int get_time(char *time_str);

#endif /* INCLUDE_INFO_H_ */
