#ifndef _COMMON_H_
#define _COMMON_H_
#include <sys/types.h>       
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "ikcp.h"
#include <errno.h>

#define MAX_KCP_NUM 32
#define MAX_CLIENT_BUF_SIZE 512

#define SUCCESS_0   0
#define FALSE_0     0
#define SUCCESS_1   1
#define FALSE_1     1

#define UDP_IP  "192.168.5.84"  //"127.0.0.1"
#define UDP_PORT 10000


#define PRINTF(fmt...)   \
    do {\
        printf("[%s]:%d:  ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)
	

typedef struct _client_data_
{
   ikcpcb *kcp; 
   char CliBuf[MAX_CLIENT_BUF_SIZE];
   struct sockaddr_in stCliAddr;
}CLIENT_DATA;

typedef struct _kcp_arg_
{
    int (*init_send_handle)();
    int (*init_recv_handle)();
	int (*init_kcp)(IUINT32 sConvNo, void *pUserData, int sWndSize, int sTransMode, IUINT32 sUpdateTime);
	IUINT32 (*iclock)();
	void (*isleep)(unsigned long millisecond);
	CLIENT_DATA client_data;
}KCP_ARG;

typedef enum{
	CLIENT_EXIT = 0,
	RECV_DATA_FAILED,
}CLIENT_FLAG;

typedef enum{
	DEFAULT_MODE = 0,
	NORMAL_MODE,
	QUICK_MODE,
}TRANS_MODE;
extern KCP_ARG kcp_arg;

#endif
