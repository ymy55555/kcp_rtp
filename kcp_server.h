#ifndef __KCP_SERVER_H__
#define __KCP_SERVER_H__
#include "common.h"

//反应当前实时数据
typedef struct _udp_server_data_
{
	int sServerFd;
	int sSysRunState;
	int sWndSize;
	int sUpdateTime;
	struct sockaddr_in stServerAddr;
	struct sockaddr_in stTransAddr;
}UDP_SERVER_DATA;

extern UDP_SERVER_DATA g_server_data;

#endif

