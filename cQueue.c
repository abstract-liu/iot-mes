/*
 * CQueue.c
 *
 *  Created on: Sep 2, 2016
 *      Author: zxx
 */

#include "cQueue.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/*构造一个空队列*/
CQueue *InitCQueue()
{
	CQueue *pqueue = (CQueue *)malloc(sizeof(CQueue));
	if(pqueue!=NULL)
	{
		pqueue->front = NULL;
		pqueue->rear = NULL;
		pqueue->size = 0;
	}
	return pqueue;
}
/*销毁一个队列*/
void DestroyCQueue(CQueue *pqueue)
{
		if(IsEmptyCQueue(pqueue)!=1)
				ClearCQueue(pqueue);
		free(pqueue);
}
/*清空一个队列*/
void ClearCQueue(CQueue *pqueue)
{
	while(IsEmptyCQueue(pqueue)!=1)
	{
		DeCQueue(pqueue);
	}
}
/*判断队列是否为空*/
int IsEmptyCQueue(CQueue *pqueue)
{
	//printf("%d %d %d\n",pqueue->front,pqueue->rear,pqueue->size);
	if(pqueue->front==NULL&&pqueue->rear==NULL&&pqueue->size==0)
		return 1;
	else
		return 0;
}

/*返回队列大小*/
int GetSizeCQueue(CQueue *pqueue)
{
		return pqueue->size;
}


CQNode EnCQueue(CQueue *pqueue,Itemt1 item)
{
		CQNode pnode = (CQNode)malloc(sizeof(CQnode));
		//int length = item->head[3];
		int i = 0;

		if(pnode != NULL)
		{
				pnode->data=(char *)malloc(strlen(item)+1);     //新开辟一块地址，存储入队数据

				memset(pnode->data,0,strlen(item)+1);
				memcpy(pnode->data,item,strlen(item)+1);

				pnode->next = NULL;

				if(IsEmptyCQueue(pqueue))
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
CQNode DeCQueue(CQueue *pqueue)
{
	CQNode pnode = pqueue->front;
	if(IsEmptyCQueue(pqueue)!=1&&pnode!=NULL)
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
