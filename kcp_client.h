#ifndef _KCP_CLIENT_H_
#define _KCP_CLIENT_H_
#include "common.h"

#define CLIENT_BIND_SERVER
#define SOCKET_RECV_NOBLOCK  //接受=收数据非阻塞

typedef struct _udp_client_data_
{
	int sClientFd;
	int sSysRunState;
	int sWndSize;
	int sUpdateTime;
	struct sockaddr_in stTransAddr;
}UDP_CLIENT_DATA;

extern UDP_CLIENT_DATA g_client_data;



#endif
