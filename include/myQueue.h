/*
 * queueStruct.h
 *
 *  Created on: Sep 2, 2015
 *      Author: openstack
 */

#ifndef QUEUE_H_
#define QUEUE_H_
#include "mythread.h"
#include "serialscreen.h"
typedef  struct ComFrame * Itemt;

typedef struct Qnode * QNode;
typedef struct Qnode
{
	Itemt data;
	QNode next;
}Qnode;


typedef struct Queue
{
	QNode front;
	QNode rear;
	int size;
}Queue;

/*构造一个空队列*/
Queue *InitQueue();

/*销毁一个队列*/
void DestroyQueue(Queue *pqueue);

/*清空一个队列*/
void ClearQueue(Queue *pqueue);


/*返回队列大小*/
int GetSizeQueue(Queue *pqueue);

/*返回队头元素*/
QNode GetFrontQueue(Queue *pqueue,Itemt *pitem);

/*返回队尾元素*/
QNode GetRearQueue(Queue *pqueue,Itemt *pitem);

/*将新元素入队*/
QNode EnQueue(Queue *pqueue,Itemt item);

/*将新元素入队      将array字符串封装成sendstr 进入队列*/
QNode EnQueuebyArray(Queue *pqueue,unsigned char *array);

#ifdef  __cplusplus  
extern "C" {  
#endif  
/*队头元素出队*/
QNode DeQueue(Queue *pqueue);
/*判断队列是否为空*/
int IsEmptyQueue(Queue *pqueue);
#ifdef  __cplusplus  
}  
#endif

//delete Node where node->item=pitem
//int deleteNode(List *pList,Item pitem);
//int DelQueue(Queue *pqueue,Itemt *pitem);
unsigned int FindQueueValue(Queue *pqueue,Itemt pitem,int length);  //查找并删除节点

int DeleteQueueNode(Queue *pqueue,Itemt pitem);

QNode FindQueueNode(Queue *pqueue,Itemt pitem);
//int deleteNode(Queue *pqueue,Itemt pitem);
/*遍历队列并对各数据项调用visit函数*/
void QueueTraverse(Queue *pqueue);

#endif /* Queue_H_ */
