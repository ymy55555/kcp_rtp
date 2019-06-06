#include "cirqueue.h"


//初始化
static int cirqueue_init(cir_pqueue *ppq)
{
      *ppq = (cir_pqueue)malloc(sizeof(cir_queue));
      if(NULL == *ppq)
      {
	    PRINTF("malloc failed");
	    return FALSE_0;
      }

      (*ppq)->front = CIRCLE_QUEUE_SIZE-1;
      (*ppq)->rear = (*ppq)->front;
	  return SUCCESS_1;
}

//判断满
static bool cirqueue_full(cir_pqueue pq)
{
      if(pq->front == pq->rear+1)
	    return SUCCESS_1;
      else
	    return FALSE_0;
}

//判断空
static bool cirqueue_empty(cir_pqueue pq)
{
      if(pq->front == pq->rear)
	    return SUCCESS_1;
      else
	    return FALSE_0;
}

//入队
static int cirqueue_insert(cir_pqueue pq,cirqueue_datatype insert_data)
{
      if(true == cirqueue_full(pq))
      {
	   // MY_PRINTF("The loop pqueue is full and the team fails to enter.\n");
	    return FALSE_0;
      }

      pq->rear = (pq->rear+1)%CIRCLE_QUEUE_SIZE;
      pq->queue_data[pq->rear] = insert_data;

      return SUCCESS_1;
}

//出队
static int cirqueue_out(cir_pqueue pq,cirqueue_datatype *out_data)
{
      if(true == cirqueue_empty(pq))
      {
	   // MY_PRINTF("circle queue is empty\n");
	    return FALSE_0;
      }

      pq->front = (pq->front+1)%CIRCLE_QUEUE_SIZE;
      *out_data = pq->queue_data[pq->front];

      return SUCCESS_1;
}

static void cirqueue_free(cir_pqueue pq)
{
	  int i = -1;
	  uuid_t uuid;
      if(SUCCESS_1 == cirqueue_empty(pq))
      {
           return;
      }
	  printf("\n---------------circle queue free---------------\n");
      for(i=(pq->front+1)%CIRCLE_QUEUE_SIZE;i!=(pq->rear+1)%CIRCLE_QUEUE_SIZE;i=(i+1)%CIRCLE_QUEUE_SIZE)
	  {
	  	
	  	uuid_parse(pq->queue_data[i].uuidBuf, uuid);
	    uuid_clear(uuid);
	  }
      free(pq);
}


//显示队列所有数据
static void cirqueue_display(cir_pqueue pq)
{
      int i;
      if(SUCCESS_1 == cirqueue_empty(pq))
      {
           return;
      }
	  printf("\n---------------circle queue data start---------------\n");
      for(i=(pq->front+1)%CIRCLE_QUEUE_SIZE;i!=(pq->rear+1)%CIRCLE_QUEUE_SIZE;i=(i+1)%CIRCLE_QUEUE_SIZE)
	  {
            // printf("cirqueue No :%d\n", i);
             //printf("     IP:%s\n", );
            // printf("   PORT:%d\n", ));
         printf("\n");
	  }
	  printf("---------------circle queue data end--------------\n");
      printf("\n");
      return;
}


CIRQUEUE_ARG  cirqueue_arg = {
	 .cirqueue_init = cirqueue_init,
	 .cirqueue_insert = cirqueue_insert,
	 .cirqueue_out = cirqueue_out,
	 .cirqueue_full = cirqueue_full,
	 .cirqueue_empty = cirqueue_empty,
	 .cirqueue_display = cirqueue_display,
	 .cirqueue_free = cirqueue_free,
	 
};


