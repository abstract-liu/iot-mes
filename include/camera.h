#ifndef __INCLUDE_CAMERA_H_
#define __INCLUDE_CAMERA_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>


/*#include "mythread.h"
#include "myQueue.h"
#include "myList.h"
#include "zigbee.h"
#include "infrared.h"
*/

#ifdef  __cplusplus  
extern "C" {  
#endif  
int get_Image();

#ifdef  __cplusplus  
}  
#endif

//typedef struct Da Data_anchi; 
//void anchi_struct_init(Data_anchi *data_anchi);
//int anchi_struct_init_db(sqlite3 *db, Data_anchi **data_anchi_p,int data_cnt);
//int anchi_cjson_datalist(cJSON *root, Data_anchi *data_anchi, int data_cnt);
//int anchi_data_collect(modbus_t *ctx, Data_anchi *data_anchi,int data_cnt);
//int anchi_send_status(int status_code);
//void *  industy_protocal_anchi(void*args);
//void * industry_protocal_anchi_thread(void *args);
#endif /* INCLUDE_DATA_COLLECT_INOVANCE_H_ */
