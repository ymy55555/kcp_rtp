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
#include <stdbool.h>
#include <uuid/uuid.h>
#include <sys/ioctl.h>

//#include "iuuid.h"
#define MAX_KCP_NUM 32
#define MAX_CLIENT_BUF_SIZE 1344

#define SUCCESS_0   0
#define FALSE_0     0
#define SUCCESS_1   1
#define FALSE_1     1

#define UDP_IP  "192.168.5.84"  //"127.0.0.1"
#define UDP_PORT 6000


#define PRINTF(fmt...)   \
    do {\
        printf("[%s]:%d:  ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)
	
typedef struct _istring_{
	char **str; 	//the PChar of string array
	size_t num; 	//the number of string
}IString;


typedef struct _kcp_arg_
{
    int (*init_send_handle)(int sSocketFd, void *sSendBuf, 
                 int sSendBufSize, struct sockaddr_in *stTransAddr);
    int (*init_recv_handle)(int sSocketFd, struct sockaddr_in *stTransAddr);
	int (*init_kcp)(IUINT32 sConvNo, void *pUserData, int sWndSize, int sTransMode, IUINT32 sUpdateTime);
	IUINT32 (*iclock)();
	void (*isleep)(unsigned long millisecond);
	int (*MySplit)(char *src, char *delim, IString* istr);
	void (*MySplitFree)(IString* istr);
	//接受到客户端发送数据标识，
	int g_sRecvFlag;
	ikcpcb *kcp;
}KCP_ARG;

typedef enum{
	DEFAULT_MODE = 0,
	NORMAL_MODE,
	QUICK_MODE,
}TRANS_MODE;

typedef struct _kcp_transfrom_data_
{	
	char uuidBuf[36];
	char DataBuf[512];
	char ClientIpBuf[15];
	int sClientPort;
}KCP_TRANSFROM_DATA;

typedef struct ikcp_seg
{
	struct IQUEUEHEAD node;
	IUINT32 conv;
	IUINT32 cmd;
	IUINT32 frg;
	IUINT32 wnd;
	IUINT32 ts;
	IUINT32 sn;
	IUINT32 una;
	IUINT32 len;
	IUINT32 resendts;
	IUINT32 rto;
	IUINT32 fastack;
	IUINT32 xmit;
	char data[1];
}IKCP_SEG;

extern KCP_ARG kcp_arg;

#endif
