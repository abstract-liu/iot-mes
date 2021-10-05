/*
 * Queue.c
 *
 *  Created on: Sep 2, 2016
 *      Author: zxx
 */

#include "myQueue.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/*构造一个空队列*/
Queue *InitQueue()
{
	Queue *pqueue = (Queue *)malloc(sizeof(Queue));
	if(pqueue!=NULL)
	{
		pqueue->front = NULL;
		pqueue->rear = NULL;
		pqueue->size = 0;
	}
	return pqueue;
}
/*销毁一个队列*/
void DestroyQueue(Queue *pqueue)
{
	if(IsEmptyQueue(pqueue)!=1)
		ClearQueue(pqueue);
	free(pqueue);
}
/*清空一个队列*/
void ClearQueue(Queue *pqueue)
{
	while(IsEmptyQueue(pqueue)!=1)
	{
		DeQueue(pqueue);
	}
}
/*判断队列是否为空*/
int IsEmptyQueue(Queue *pqueue)
{
	//printf("%d %d %d\n",pqueue->front,pqueue->rear,pqueue->size);
	if(pqueue->front==NULL&&pqueue->rear==NULL&&pqueue->size==0)
		return 1;
	else
		return 0;
}

/*返回队列大小*/
int GetSizeQueue(Queue *pqueue)
{
	return pqueue->size;
}

/*返回队头元素*/
QNode GetFrontQueue(Queue *pqueue,Itemt *pitem)
{
		if(IsEmptyQueue(pqueue)!=1&&pitem!=NULL)
		{
				*pitem = pqueue->front->data;
		}
		return pqueue->front;
}
/*返回队尾元素*/
QNode GetRearQueue(Queue *pqueue,Itemt *pitem)
{
		if(IsEmptyQueue(pqueue)!=1&&pitem!=NULL)
		{
				*pitem = pqueue->rear->data;
		}
		return pqueue->rear;
}
QNode EnQueue(Queue *pqueue,Itemt item)
{
	QNode pnode = (QNode)malloc(sizeof(Qnode));

	if(pnode != NULL)
	{
		pnode->data=(struct ComFrame*)malloc(sizeof(struct ComFrame));
		pnode->next = NULL;

		pnode->data->frmlen	= item->frmlen;
		memcpy(pnode->data->data,item->data,item->frmlen);


		if(IsEmptyQueue(pqueue))
		{
			pqueue->front = pnode;
		}
		else
		{
			pqueue->rear->next = pnode;
		}

		pqueue->rear = pnode;
		pqueue->size++;
	}
	return pnode;
}


/*队头元素出队*/
QNode DeQueue(Queue *pqueue)
{
	QNode pnode = pqueue->front;
	if( IsEmptyQueue(pqueue) !=1 && pnode != NULL)
	{
		pqueue->size--;
		pqueue->front = pnode->next;
		free(pnode->data);
		free(pnode);
		if(pqueue->size==0)
			pqueue->rear = NULL;
	}
	return pqueue->front;
}


/*遍历队列并对各数据项调用visit函数*/
void QueueTraverse(Queue *pqueue)
{
	QNode pnode = pqueue->front;
	int i		= pqueue->size;
	printf("i = %d",i);
}
