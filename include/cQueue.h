/*
 * queueStruct.h
 *
 *  Created on: Sep 2, 2015
 *      Author: openstack
 */

#ifndef C_QUEUE_H_
#define C_QUEUE_H_
#include "mythread.h"
#include "serialscreen.h"
typedef  char * Itemt1;

typedef struct CQnode * CQNode;
typedef struct CQnode
{
	Itemt1 data;
	CQNode next;
}CQnode;

typedef struct CQueue
{
	CQNode front;
	CQNode rear;
	int size;
}CQueue;

/*构造一个空队列*/
CQueue *InitCQueue();

/*销毁一个队列*/
void DestroyCQueue(CQueue *pqueue);

/*清空一个队列*/
void ClearCQueue(CQueue *pqueue);

#ifdef  __cplusplus  
extern "C" {  
#endif  
/*判断队列是否为空*/
int IsEmptyCQueue(CQueue *pqueue);
/*队头元素出队*/
CQNode DeCQueue(CQueue *pqueue);
/*将新元素入队*/
CQNode EnCQueue(CQueue *pqueue,Itemt1 item);
#ifdef  __cplusplus  
}  
#endif




/*返回队列大小*/
int GetSizeCQueue(CQueue *pqueue);




#endif /* CQueue_H_ */
