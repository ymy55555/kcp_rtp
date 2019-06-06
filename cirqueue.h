#ifndef _CIRQUEUE_H_
#define _CIRQUEUE_H_
#include "common.h"

#define CIRCLE_QUEUE_SIZE 1024

//队列数据
typedef struct cirqueue_data
{
	char uuidBuf[36];
	char DataBuf[512];
	char ClientIpBuf[15];
	int sClientPort;
}QUEUE_DATA;

typedef QUEUE_DATA cirqueue_datatype;
typedef struct cirqueue
{
      cirqueue_datatype queue_data[CIRCLE_QUEUE_SIZE];
      int rear,front;
}cir_queue,*cir_pqueue;


//队列需要的变量和函数
typedef struct _cirqueue_arg_
{
	 int (*cirqueue_init)(cir_pqueue *ppq);
	 int (*cirqueue_insert)(cir_pqueue pq,cirqueue_datatype insert_data);
	 int (*cirqueue_out)(cir_pqueue pq,cirqueue_datatype *out_data);
	 bool (*cirqueue_full)(cir_pqueue pq);
	 bool (*cirqueue_empty)(cir_pqueue pq);
	 void (*cirqueue_display)(cir_pqueue pq);
	 void (*cirqueue_free)(cir_pqueue pq);
	 cir_pqueue pqueue;
}CIRQUEUE_ARG;
extern CIRQUEUE_ARG  cirqueue_arg;


#endif
