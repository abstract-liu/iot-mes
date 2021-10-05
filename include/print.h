/*
 * print.h
 *
 *  Created on: Oct 14, 2020
 *      Author: lj
 */


#ifndef INCLUDE_PRINT_H_
#define INCLUDE_PRINT_H_

#include "mosquitto.h"

void *th_print(void);//线程
void print_callback(struct mosquitto *mosq, void *userdata,
		const struct mosquitto_message *msg);//sub后需要做的处理
void print_cnt_callback(struct mosquitto *mosq, void *userdata, int result);//sub重连


void print_succes_back(struct mosquitto *mosq, cJSON *root);

#endif /* INCLUDE_STATUS_H_ */
