#include "common.h"
#include "kcp_client.h"
#include "kcp_server.h"
#include "cirqueue.h"
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

//UDP发送的地址来自接收的客户端的地址
static int init_send_handle(int sSocketFd, void *sSendBuf, 
                 int sSendBufSize, struct sockaddr_in *stTransAddr)
{
	int sRet = -1;
#if 1
	IString istr;
	if(kcp_arg.MySplit((char *)sSendBuf, (char *)"|", &istr))
	{
		printf("____________sendto___uuid:%s\n", istr.str[0]);
		printf("____________sendto____ip:%s  port:%d\n", istr.str[1], atoi(istr.str[2]));
		kcp_arg.MySplitFree(&istr);
	}
#endif  
	sRet = sendto(sSocketFd, sSendBuf, sSendBufSize, 0,
		(struct sockaddr *)stTransAddr, sizeof(struct sockaddr_in));
	if(0 == sRet)
	{
		PRINTF("send failed,Receiver is exit.\n");
		//...
		return FALSE_0;
	}else if(-1 == sRet)
	{
		PRINTF("send data failed.\n");
		//...
		printf("errno is %d\n", errno);
		return FALSE_0;
	}
	return SUCCESS_1;
}

//待分离...
static int init_recv_handle(int sSocketFd, struct sockaddr_in *stTransAddr)
{
     int sRet = -1;
	 char *sRecvBuf;
	 IString istr;
	 cirqueue_datatype stCirqueueData;
	 memset(&stCirqueueData, 0, sizeof(stCirqueueData));
	 sRecvBuf = (void *)malloc(MAX_CLIENT_BUF_SIZE);
	 socklen_t addr_len = sizeof(struct sockaddr_in);
	 ikcp_update(kcp_arg.kcp, kcp_arg.iclock());
	 sRet = recvfrom(sSocketFd, (void *)sRecvBuf,MAX_CLIENT_BUF_SIZE, 0,
	 	                         (struct sockaddr *)stTransAddr, &addr_len);
	 if(-1 == sRet)
	 {
		PRINTF("recvfrom data failed.   errno %d\n", errno);
		//...
		return FALSE_0;
	 }
	 	 
#if 1

	if(kcp_arg.MySplit((char *)sRecvBuf, (char *)"|", &istr))
	{
		 memcpy(stCirqueueData.uuidBuf, istr.str[0], 36);
		 memcpy(stCirqueueData.ClientIpBuf, istr.str[1], 15);
		 stCirqueueData.sClientPort = atoi(istr.str[2]);
		 kcp_arg.MySplitFree(&istr); 
	 }
	 printf("-----------recv-client uuid:%s\n", stCirqueueData.uuidBuf);
	 
	 printf("-----------recv-client ip:%s---port:%d\n", stCirqueueData.ClientIpBuf, 
	 	                                                stCirqueueData.sClientPort);
	 cirqueue_arg.cirqueue_insert(cirqueue_arg.pqueue, stCirqueueData);
#endif	 
	 //kcp接收到下层协议UDP传进来的数据底层数据buffer转换成kcp的数据包格式
	 sRet = -1;
	 sRet = ikcp_input(kcp_arg.kcp, sRecvBuf, MAX_CLIENT_BUF_SIZE);
	 //PRINTF("ikcp_input:%s\n", sRecvBuf);
     if (sRet < 0) 
	 {
        PRINTF("ikcp_input error:, ret :%d\n", sRet);
		return FALSE_0;
     }
	 sRet = -1;
     while(1)
	 {
	    kcp_arg.isleep(1);
        sRet = ikcp_recv(kcp_arg.kcp, sRecvBuf, MAX_CLIENT_BUF_SIZE);
        if(sRet < 0){
            break;
        }
		   ikcp_send(kcp_arg.kcp, sRecvBuf, sRet);//临时数据回射
    }
	return SUCCESS_1;
}

int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    if(!user)
    {
       PRINTF("user is null.\n");
	   return 0;
	}

#if defined(DEFINE_SERVER)
	if(SUCCESS_1 != init_send_handle(g_server_data.sServerFd,
		         user, MAX_CLIENT_BUF_SIZE, &g_server_data.stTransAddr))
	{
	   PRINTF("Send failed.\n");	
	}
#elif defined(DEFINE_CLIENT)
	if(SUCCESS_1 != init_send_handle(g_client_data.sClientFd,
		           user, MAX_CLIENT_BUF_SIZE, &g_client_data.stTransAddr))
	{
	   PRINTF("Send failed.\n");	
	}
#endif
	return 0;
}

static int init_kcp(IUINT32 sConvNo, void *pUserData, int sWndSize, int sTransMode, IUINT32 sUpdateTime)
{

	// 创建两个端点的 kcp对象，第一个参数 conv是会话编号，同一个会话需要相同
	// 最后一个是 user参数，用来传递标识
	kcp_arg.kcp = ikcp_create(sConvNo, pUserData);//多个客户端时kcp的情况待分析。。。

   //待考虑线程巡航检测更新。。。
    //IUINT32 current = iclock();
	//IUINT32 slap = current + sUpdateTime;
	// 设置kcp的下层输出，这里为 udp_output，模拟udp网络输出函数
	kcp_arg.kcp->output = kcp_output;
	/*
	 配置窗口大小：平均延迟200ms，每20ms发送一个包，而考虑到丢包重发,
	 设置最大收发窗口为128KCP默认为32，即可以接收最大为32*MTU=43.75kB。
	 KCP采用update的方式，更新间隔为10ms，那么KCP限定了你最大传输速率为4375kB/s，
	 在高网速传输大内容的情况下需要调用ikcp_wndsize调整接收与发送窗口。
	 可以检测网络传输平均值，定期更新窗口大小
	*/
	ikcp_wndsize(kcp_arg.kcp, sWndSize, sWndSize);
	switch(sTransMode)
	{
	   case 0:
		   // 默认模式
		   ikcp_nodelay(kcp_arg.kcp, 0, 10, 0, 0);
	   break;

	   case 1:
		   // 普通模式，关闭流控等
		   ikcp_nodelay(kcp_arg.kcp, 0, 10, 0, 1);
	   break;

	   case 2:
		   // 启动快速模式
		   // 第二个参数 nodelay-启用以后若干常规加速将启动
		   // 第三个参数 interval为内部处理时钟，默认设置为 10ms
		   // 第四个参数 resend为快速重传指标，设置为2
		   // 第五个参数 为是否禁用常规流控，这里禁止
		   ikcp_nodelay(kcp_arg.kcp, 1, 10, 2, 1);
		   kcp_arg.kcp->rx_minrto = 10;
		   kcp_arg.kcp->fastresend = 1;
	   break;
	}
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
 
 
/** \Split string by a char
 *
 * \param  src:the string that you want to split
 * \param  delim:split string by this char
 * \param  istr:a srtuct to save string-array's PChar and string's amount.
 * \return  whether or not to split string successfully
 *
 */
static int MySplit(char *src, char *delim, IString* istr)
{
    int i,ret = 1;
    char *str = NULL, *p = NULL;
 
    (*istr).num = 1;
	str = (char*)calloc(strlen(src)+1,sizeof(char));
	if (str == NULL) return 0;
    (*istr).str = (char**)calloc(1,sizeof(char *));
    if ((*istr).str == NULL) {ret = 0;goto exit;}
    strcpy(str,src);
 
	p = strtok(str, delim);
	(*istr).str[0] = (char*)calloc(strlen(p)+1,sizeof(char));
	if ((*istr).str[0] == NULL) return 0;
 	strcpy((*istr).str[0],p);
	p = strtok(NULL, delim);
	for(i = 1;p;i++)
    {
        (*istr).num++;
		
        (*istr).str = (char**)realloc((*istr).str,(i+1)*sizeof(char *));
        if ((*istr).str == NULL) {ret = 0;goto exit;}
        (*istr).str[i] = (char*)calloc(strlen(p)+1,sizeof(char));
        if ((*istr).str[0] == NULL){ret = 0;goto exit;}
        strcpy((*istr).str[i],p);
	p = strtok(NULL, delim);
    }
exit:
    free(str);
    str = p = NULL;
 
    return ret;
}
static void MySplitFree(IString* istr)
{
	 int i = -1;
     for (i=0;i<(*istr).num;i++)
         free((*istr).str[i]);
      free((*istr).str);
}

 

KCP_ARG kcp_arg = {
	.init_send_handle = init_send_handle,
    .init_recv_handle = init_recv_handle,
	.init_kcp = init_kcp,
    .iclock = iclock,
    .isleep = isleep,
    .g_sRecvFlag = FALSE_0,

    .MySplit = MySplit,
    .MySplitFree = MySplitFree,
};

