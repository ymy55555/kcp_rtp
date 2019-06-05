#include "common.h"    
#include "kcp_client.h"
#include "cirqueue.h"


    //int sClientFd;
	//int sSysRunState;
	//int sWndSize;
	//int sUpdateTime;
	//struct sockaddr_in stTransAddr;

UDP_CLIENT_DATA g_client_data = { 
    .sClientFd = -1,
	.stTransAddr = {},
	.sSysRunState = SUCCESS_1,
	.sWndSize = 128,
	.sUpdateTime = 20,//ms
};

static void config_client_param()
{
    g_client_data.stTransAddr.sin_family = AF_INET;
    g_client_data.stTransAddr.sin_port = htons(UDP_PORT);
    g_client_data.stTransAddr.sin_addr.s_addr = inet_addr(UDP_IP);
}

static int init_client()
{
    g_client_data.sClientFd = socket(AF_INET,SOCK_DGRAM,0);
    if (-1 == g_client_data.sClientFd)
	{
	    PRINTF("Socket failed.\n");
		close(g_client_data.sClientFd);
		return FALSE_0;
	}
    return SUCCESS_1;
   
}

static void free_client()
{
    
    close(g_client_data.sClientFd);
	cirqueue_arg.cirqueue_free(cirqueue_arg.pqueue);
}

int main(int argc, char const *argv[])
{
	 int sRet = -1;
	 config_client_param();
	 if(SUCCESS_1 != init_client())
	 {
        PRINTF("Init udp client failed.\n");
		return FALSE_0;
	 }
	 kcp_arg.init_kcp(0x002, (void *)"0", g_client_data.sWndSize,
	                        DEFAULT_MODE, g_client_data.sUpdateTime);
	 char buf[255];
	 while(1)
     {
          
		  kcp_arg.isleep(1);
		  buf[0] = '\0';
          PRINTF("Plz input data:");
          gets(buf);
		  ikcp_update(kcp_arg.kcp, kcp_arg.iclock());
		  sRet = ikcp_send(kcp_arg.kcp, buf, strlen(buf));
	      if(sRet < 0){
				PRINTF("client send failed.\n");
				continue;
		  }
		  ikcp_update(kcp_arg.kcp, kcp_arg.iclock());
          //sendto(g_client_data.sClientFd,buf,strlen(buf)+1,0,(struct sockaddr*)& g_client_data.stTransAddr,sizeof( g_client_data.stTransAddr));
          //if(0 == strcmp(buf,"q")) break;

         // recvfrom(g_client_data.sClientFd,buf,sizeof(buf),0,(struct sockaddr*)& g_client_data.stTransAddr,&addr_len);
         // printf("Recv:%s\n",buf);
         // if(0 == strcmp(buf,"q")) break;
   
     }
	 free_client();
     return 0;
}
