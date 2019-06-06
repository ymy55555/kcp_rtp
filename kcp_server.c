#include "common.h"
#include "kcp_server.h"
#include "cirqueue.h"

    //int sServerFd;
	//int sSysRunState;
	//int sWndSize;
	//int sUpdateTime;
	//struct sockaddr_in stServerAddr;

UDP_SERVER_DATA g_server_data = {
    .sServerFd = -1,  
    .stServerAddr = {},
    .stTransAddr = {},
	.sSysRunState = SUCCESS_1,
	.sWndSize = 128,
	.sUpdateTime = 20,//ms
};

static void config_server_param()
{
	g_server_data.stServerAddr.sin_family = AF_INET;
	g_server_data.stServerAddr.sin_port = htons(UDP_PORT);
	g_server_data.stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(UDP_IP);

}
static int init_udp_server()
{
    int sRet = -1;
    g_server_data.sServerFd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == g_server_data.sServerFd)
	{
	    PRINTF("Socket failed.\n");
		close(g_server_data.sServerFd);
		return FALSE_0;
	}
    sRet = bind(g_server_data.sServerFd, (struct sockaddr*)&g_server_data.stServerAddr,
                sizeof(g_server_data.stServerAddr));
	if(SUCCESS_0 != sRet)
	{
		PRINTF("Bind falied.__%d\n", errno);
		close(g_server_data.sServerFd);
		return FALSE_0;
	}
	return SUCCESS_1;
}

static int init_server()
{
    if(SUCCESS_1 != init_udp_server())
	{
	    PRINTF("Server socket failed.\n");
		return FALSE_0;
	}
	if(SUCCESS_1 != cirqueue_arg.cirqueue_init(&cirqueue_arg.pqueue))
    {	
        PRINTF("Init circle queue failed.\n");
        return FALSE_0;
    }
	return SUCCESS_1;
}

static void free_server()
{
   close( g_server_data.sServerFd);
   cirqueue_arg.cirqueue_free(cirqueue_arg.pqueue);
}

/*
   struct pollfd *poll_table

   poll_entry->fd = c->fd;
				poll_entry->events = POLLIN;
				poll_entry->revents= 0;
				ret = poll(poll_table, 1, 0);
				if(ret <= -1)
				{ERR}
	for(i=0; i<(MAX_TCP_USERS + 1); i++)
		{
			c = &s_stSocketCtx[i];
			if(c->fd == -1)
				continue;
            if(c->poll_entry && c->poll_entry->revents)
			{
				if(c->handle(c))
				{
					SAVE_CLOSESOCKET(c->fd);
				}
			}
        }
	 循环利用poll检测UDP 的fd
	 发送做给函数？给线程？做地址识别？根据接收特定发送？
	 先做交互处理

*/

int main(int argc, char const *argv[])
{
	 config_server_param();
	 if(SUCCESS_1 != init_server())
	 {
	    PRINTF("Init server failed.\n");
		return FALSE_0;
	 }
	 kcp_arg.init_kcp(0x001, (void *)"1111", g_server_data.sWndSize, DEFAULT_MODE, g_server_data.sUpdateTime);
	 while(1)
     {
	      kcp_arg.isleep(1);
		  PRINTF("____________1__________\n");
         // memset(&g_server_data.stTransAddr, 0, sizeof(g_server_data.stTransAddr));
          (void)kcp_arg.init_recv_handle(g_server_data.sServerFd, &g_server_data.stTransAddr);
		// printf("___________please input data:\n");
		// gets(buf);
         //待定义发送规则
         //发送地址来自接收地址
		 //(void)kcp_arg.init_send_handle(g_server_data.sServerFd, buf, sizeof(buf), &g_server_data.stTransAddr);
		 PRINTF("____________2__________\n");
     }  
	 free_server();
     return 0;
}
