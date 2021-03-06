#include "common.h"    
#include "kcp_client.h"
#include "cirqueue.h"

UDP_CLIENT_DATA g_client_data = { 
    .sClientFd = -1,
	.stTransAddr = {},
	.sSysRunState = SUCCESS_1,
	.sWndSize = 128,
	.sUpdateTime = 20,//ms
};

static void config_net_param()
{
    g_client_data.stTransAddr.sin_family = AF_INET;
    g_client_data.stTransAddr.sin_port = htons(UDP_PORT);
    g_client_data.stTransAddr.sin_addr.s_addr = inet_addr(UDP_IP);
}

static int config_transfrom_data(char *ClientDataBuf)
{
	uuid_t	uuid;  
	int sRet = -1;
	uuid_clear(uuid);
	char TmpIpBuf[15] = {0};
	KCP_TRANSFROM_DATA stClientData;
	sRet = uuid_generate_time_safe(uuid);
	if(SUCCESS_0 != sRet)
	{
	   PRINTF("uuid is creat failed.\n");
	   return FALSE_0;
	}
	uuid_unparse(uuid, stClientData.uuidBuf);
	//printf("Client creat uuid:%s\n", stClientData->uuidBuf);
	if(!inet_ntop(AF_INET, (const void *)&g_client_data.stTransAddr.sin_addr.s_addr, 
		   TmpIpBuf, sizeof(TmpIpBuf))) 
	{
        PRINTF("Get client ip is failed .\n");
		return FALSE_0;
    }		   
	strcpy(stClientData.ClientIpBuf, TmpIpBuf);
	sprintf(ClientDataBuf, "%s|%s",	stClientData.ClientIpBuf,
								         stClientData.uuidBuf);
	                                //stClientData.DataBuf //真正要传输数据
	PRINTF("______________send buf: %s:\n", ClientDataBuf);
	return SUCCESS_1;
}

static int init_client()
{
    g_client_data.sClientFd = socket(AF_INET,SOCK_DGRAM,0);
    if (-1 == g_client_data.sClientFd)
	{
	    PRINTF("Socket failed.\n");
		return FALSE_0;
	}
#if defined(SOCKET_RECV_NOBLOCK)//连接非阻塞
	int sNoBlock = 1;
    if(ioctl( g_client_data.sClientFd, FIONBIO, &sNoBlock) < 0)
	{		
	   PRINTF("ioctl FIONBIO failed.\n"); 
       return FALSE_0;   
	}
#endif
    return SUCCESS_1;
   
}

static void free_client()
{
    
    close(g_client_data.sClientFd);
	cirqueue_arg.cirqueue_free(cirqueue_arg.pqueue);
	ikcp_release(kcp_arg.kcp);
}

static void main_loop(char ClientDataBuf[MAX_CLIENT_BUF_SIZE])
{
	int sRet = -1;
    while(g_client_data.sSysRunState)
	{
		 
	     kcp_arg.isleep(1);
		 sRet = ikcp_send(kcp_arg.kcp, ClientDataBuf, MAX_CLIENT_BUF_SIZE);
		 ikcp_update(kcp_arg.kcp, kcp_arg.iclock());
		 if(sRet < 0)
		 {
			   PRINTF("client send failed.\n");
			   continue;
		 }
		(void)kcp_arg.init_recv_handle(g_client_data.sClientFd, &g_client_data.stTransAddr);
#if 1//test
		char GetState;
		printf("continue please press ENTER.\n");
		GetState = getchar();
		if(GetState != '\n')break;
#endif
  
	}

}

int main(int argc, char const *argv[])
{
	 
	 char ClientDataBuf[MAX_CLIENT_BUF_SIZE];
	 config_net_param();
	 if(SUCCESS_1 != init_client())
	 {
        PRINTF("Init udp client failed.\n");
		free_client();
		return FALSE_0;
	 }
	 //Please initialize the network parameters before
	 if(SUCCESS_1 != config_transfrom_data(ClientDataBuf))
	 {
		PRINTF("Client config transfrom data failed.\n");
	    free_client();
	    return FALSE_0;
	 }

	 kcp_arg.init_kcp(AF_INET, (void *)ClientDataBuf, g_client_data.sWndSize,
	                                  NORMAL_MODE, g_client_data.sUpdateTime);
	 main_loop(ClientDataBuf);
	 free_client();
     return 0;
}
