#include "common.h"
#include "kcp_client.h"
#include "kcp_server.h"
/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;	
}

/* get clock in millisecond 64 */
static inline IINT64 iclock64(void)
{
	long s, u;
	IINT64 value;
	itimeofday(&s, &u);
	value = ((IINT64)s) * 1000 + (u / 1000);
	return value;
}

static inline IUINT32 iclock()
{
	return (IUINT32)(iclock64() & 0xfffffffful);
}

//发送接收待分离
static int init_send_handle()
{
#if 0
	int sRet = -1;
	sprintf(kcp_arg.client_data.CliBuf, "_%s_", (char *)"it's a client msg");
	//memset(&kcp_arg.client_data.stCliAddr, 0, sizeof(struct sockaddr_in));
	printf("______%d\n", g_client_data.sClientFd);
	sRet = sendto(g_client_data.sClientFd, (const void *)kcp_arg.client_data.CliBuf,
	MAX_CLIENT_BUF_SIZE, 0, (const struct sockaddr*)&kcp_arg.client_data.stCliAddr, 
	sizeof(kcp_arg.client_data.stCliAddr));
	if(0 == sRet)
	{
		PRINTF("Client exit.\n");
		//...
		return CLIENT_EXIT;
	}else if(-1 == sRet)
	{
		PRINTF("Client send failed.\n");
		//...
		
		printf("__errno_______%d_\n", errno);
		return RECV_DATA_FAILED;
	}
#endif
	return SUCCESS_1;
}


static int init_recv_handle()
{
#if 1
    int sRet = -1;
	//memset(&kcp_arg.client_data.stCliAddr, 0, sizeof(struct sockaddr_in));
	sRet = recvfrom(g_server_data.sServerFd, kcp_arg.client_data.CliBuf,
		MAX_CLIENT_BUF_SIZE, 0, (struct sockaddr *)&kcp_arg.client_data.stCliAddr, 
	    (socklen_t*)sizeof(kcp_arg.client_data.stCliAddr));
	if(0 == sRet)
	{
       PRINTF("Client exit.\n");
	   //...
	   return CLIENT_EXIT;
	}else if(-1 == sRet)
	{
       PRINTF("Client recvfrom failed.");
	   //...
	   return RECV_DATA_FAILED;
    }

	//kcp接收到下层协议UDP传进来的数据底层数据buffer转换成kcp的数据包格式
	ikcp_input(kcp_arg.client_data.kcp, kcp_arg.client_data.CliBuf, sRet);
#endif
	return SUCCESS_1;
}

int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
#if 0
    PRINTF("-------------udp_output--------------\n");
	
	init_send_handle();
#endif
	return 0;
}

static int init_kcp(IUINT32 sConvNo, void *pUserData, int sWndSize, int sTransMode, IUINT32 sUpdateTime)
{

	// 创建两个端点的 kcp对象，第一个参数 conv是会话编号，同一个会话需要相同
	// 最后一个是 user参数，用来传递标识
	ikcpcb *kcp = ikcp_create(sConvNo, pUserData);

//待考虑线程巡航检测更新
    //IUINT32 current = iclock();
	//IUINT32 slap = current + sUpdateTime;
	// 设置kcp的下层输出，这里为 udp_output，模拟udp网络输出函数
	kcp->output = kcp_output;
	/*
	 配置窗口大小：平均延迟200ms，每20ms发送一个包，而考虑到丢包重发,
	 设置最大收发窗口为128KCP默认为32，即可以接收最大为32*MTU=43.75kB。
	 KCP采用update的方式，更新间隔为10ms，那么KCP限定了你最大传输速率为4375kB/s，
	 在高网速传输大内容的情况下需要调用ikcp_wndsize调整接收与发送窗口。
	 可以检测网络传输平均值，定期更新窗口大小
	*/
	ikcp_wndsize(kcp, sWndSize, sWndSize);
	switch(sTransMode)
	{
	   case 0:
		   // 默认模式
		   ikcp_nodelay(kcp, 0, 10, 0, 0);
	   break;

	   case 1:
		   // 普通模式，关闭流控等
		   ikcp_nodelay(kcp, 0, 10, 0, 1);
	   break;

	   case 2:
		   // 启动快速模式
		   // 第二个参数 nodelay-启用以后若干常规加速将启动
		   // 第三个参数 interval为内部处理时钟，默认设置为 10ms
		   // 第四个参数 resend为快速重传指标，设置为2
		   // 第五个参数 为是否禁用常规流控，这里禁止
		   ikcp_nodelay(kcp, 1, 10, 2, 1);
	   break;
	}
//将来用队列实现多客户端，或者ringbuf
	kcp_arg.client_data.kcp = kcp;
	return SUCCESS_1;

}


/* sleep in millisecond */
static inline void isleep(unsigned long millisecond)
{
	//#ifdef __unix 	/* usleep( time * 1000 ); */
	//struct timespec ts;
	//ts.tv_sec = (time_t)(millisecond / 1000);
	//ts.tv_nsec = (long)((millisecond % 1000) * 1000000);
	/*nanosleep(&ts, NULL);*/
	usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
	//#elif defined(_WIN32)
	//Sleep(millisecond);
	//#endif
}

KCP_ARG kcp_arg = {
	.init_send_handle = init_send_handle,
    .init_recv_handle = init_recv_handle,
	.init_kcp = init_kcp,
    .iclock = iclock,
    .isleep = isleep,
    .client_data = {
         .CliBuf[0] = '\0',
	},
};


