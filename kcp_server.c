#include "common.h"
#include "kcp_server.h"

UDP_SERVER_DATA g_server_data = {
	.sServerFd = -1,
	.sSysRunState = SUCCESS_1,
	.sWndSize = 128,
	.sUpdateTime = 20,//ms
};

static int init_udp_server()
{
	memset(&g_server_data.sServerAddr, 0, sizeof(struct sockaddr));
	g_server_data.sServerFd = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
	if (-1 == g_server_data.sServerFd)
	{
	    PRINTF("Socket failed.\n");
		close(g_server_data.sServerFd);
		return FALSE_0;
	}
	g_server_data.sServerAddr.sin_family = AF_INET;
#if defined(UDP_IP)
    g_server_data.sServerAddr.sin_addr.s_addr = inet_addr((const char *)UDP_IP);;
#else
	g_server_data.sServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
     g_server_data.sServerAddr.sin_port = htons(UDP_PORT);
	//bind port to socket
	if(SUCCESS_0 != bind(g_server_data.sServerFd,
		(const struct sockaddr *)&g_server_data.sServerAddr, sizeof(g_server_data.sServerAddr)))
	{
	    PRINTF("Bind falied.\n");
		close(g_server_data.sServerFd);
		return FALSE_0;
	}

	return SUCCESS_1;
}

#if 0
static void kcp_server_free()
{

}

#endif

//待对特殊情况处理
int main(int argc, char *argv[])
{
    int sRet = -1;
    if(SUCCESS_1 != init_udp_server())
    {
          PRINTF("Init udp server failed.\n");
		  exit(1);
	}
	kcp_arg.init_kcp(0x11111111, (void *)"0", g_server_data.sWndSize, DEFAULT_MODE, g_server_data.sUpdateTime);
	while(g_server_data.sSysRunState)
	{
		kcp_arg.isleep(1);
        ikcp_update(kcp_arg.client_data.kcp, kcp_arg.iclock());
		kcp_arg.init_recv_handle();
		while(1)
		{
		    kcp_arg.isleep(1);
            sRet = ikcp_recv(kcp_arg.client_data.kcp, kcp_arg.client_data.CliBuf, MAX_CLIENT_BUF_SIZE);
            if(sRet < 0){
                break;
            }
            PRINTF("ikcp_recv:%s\n", kcp_arg.client_data.CliBuf);
        }
		
	}
	return 0;
}









