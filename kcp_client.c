#include "common.h"
#include "kcp_client.h"

UDP_CLIENT_DATA g_client_data = {
	.sClientFd = -1,
	.sSysRunState = SUCCESS_1,
	.sWndSize = 128,
	.sUpdateTime = 20,//ms
};


static int init_udp_client()
{
	memset(&g_client_data.sClientAddr, 0, sizeof(struct sockaddr));
	g_client_data.sClientFd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
	if (-1 == g_client_data.sClientFd)
	{
	    PRINTF("Socket failed.\n");
		close(g_client_data.sClientFd);
		return FALSE_0;
	}
	

	g_client_data.sClientAddr.sin_family = AF_INET;
#if defined(UDP_IP) && defined(CLIENT_BIND_SERVER)
    g_client_data.sClientAddr.sin_addr.s_addr = inet_addr((const char *)UDP_IP);;
#else
	g_client_data.sClientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
     g_client_data.sClientAddr.sin_port = htons(UDP_PORT);

#if defined(CLIENT_BIND_SERVER)
	if(SUCCESS_0 != bind(g_client_data.sClientFd,
		(struct sockaddr *)&g_client_data.sClientAddr, sizeof(g_client_data.sClientAddr)))
	{
	    PRINTF("Bind falied.\n");
		close(g_client_data.sClientFd);
		return FALSE_0;
	}	
#endif
	return SUCCESS_1;
}

int main(int argc, char *argv[])
{   
    int sRet = -1;
    if(SUCCESS_1 != init_udp_client())
    {
          PRINTF("Init udp client failed.\n");
		  exit(1);
	}
	kcp_arg.init_kcp(0x55555555, (void *) kcp_arg.client_data.CliBuf, g_client_data.sWndSize, DEFAULT_MODE, g_client_data.sUpdateTime);
    while(g_client_data.sSysRunState) 
    {
		kcp_arg.isleep(1);
		ikcp_update(kcp_arg.client_data.kcp, kcp_arg.iclock());
		sprintf(kcp_arg.client_data.CliBuf, "%s", (char *)"123456");
		sRet = ikcp_send(kcp_arg.client_data.kcp, kcp_arg.client_data.CliBuf, MAX_CLIENT_BUF_SIZE);
		ikcp_update(kcp_arg.client_data.kcp, kcp_arg.iclock());
		if(sRet < 0)
		{
			break;
		}
		//PRINTF("ikcp_recv:%s\n", kcp_arg.client_data.CliBuf);
		//sleep(1);
	}
	return 0;
}


