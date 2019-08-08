static unsigned int get_randdom_seq(void)
{
   unsigned  int  seed;
   srand((unsigned)time(NULL));  
   seed = 1 + (unsigned int) (rand()%(0xFFFF)); 
   
   return seed;
}

static unsigned int  get_timestamp()
{
    struct timeval tv_date;
    gettimeofday( &tv_date, NULL );
    return( (unsigned int) tv_date.tv_sec * 1000000 + (unsigned int) tv_date.tv_usec );
}

//获取I帧RTMP extraldata
void GetRtmpData(VENC_STREAM_S *stStream, unsigned char *pExtraData)
{
#define PACK_ADDR(i) (stStream->pstPack[i].pu8Addr + stStream->pstPack[i].u32Offset)
#define PACK_LEN(i)  (stStream->pstPack[i].u32Len - stStream->pstPack[i].u32Offset)

	unsigned char sExtraData[128]  = {0};
	sExtraData[0] = 0x01;
	sExtraData[1] = PACK_ADDR(0)[5];
	sExtraData[2] = PACK_ADDR(0)[6];
	sExtraData[3] = PACK_ADDR(0)[7];
	sExtraData[4] = 0xff;
	sExtraData[5] = 0xe1;
	sExtraData[6] = (PACK_LEN(0) - 4) >> 8;
	sExtraData[7] = PACK_LEN(0)  - 4;
	memcpy(&sExtraData[8],	PACK_ADDR(0) + 4, PACK_LEN(0) - 4);
	sExtraData[PACK_LEN(0) - 4 + 8] = 0x01;
	sExtraData[PACK_LEN(0) - 4 + 8 + 1] = (PACK_LEN(1) - 4) >> 8;
	sExtraData[PACK_LEN(0) - 4 + 8 + 2] = PACK_LEN(1) - 4;
	memcpy(&sExtraData[PACK_LEN(0) - 4 + 8 + 3], PACK_ADDR(1) + 4, PACK_LEN(1) - 4);
	sExtraData = sExtraData;
}

//检测当前地址是否为起始码起始地址 返回结果
 static int CheckStartCode(unsigned char *Buf, int sNum)
 {
	 int i = -1;
	 for(i = 0;i < sNum - 1;i++)
	 {
		 Buf[0] += Buf[i+1];
	 }
	 return Buf[0];
 }

//检测起始码长度 返回结果
static int JudgeNaluStartCodeLen(unsigned char *Buf)
{
	if(0x01 == CheckStartCode(Buf, 3)) //00 00 01
	{
		return 3;
	} 
	else//00 00 00 01
	{
		if (0x01 == CheckStartCode (Buf, 4))  
		{ 
			return 4;
		}
		else 
		{
			return FALSE_0;
		}
	}
}


/*
*pStreamBuf包含个多包，起始码不固定传输， H264文件的编码使用
*stNalu传入前需要申请地址 待完善。。。
*/
int GetOnceNalu_v2(unsigned char *pStreamBuf,
                                         int sStreamSize,
                                          NALU_t *stNalu)
{
	int sTmpCount = 0;
    int sStartCodeFlag = 0;
	static int sSendpackCount = 0;
    sStartCodeFlag= JudgeNaluStartCodeLen(pStreamBuf);
	if(FALSE_0 == sStartCodeFlag)
	{
	   PRINTF("not find start code.\n");
       return FALSE_0;
	}
	stNalu->startcodeprefix_len = sStartCodeFlag;
	sSendpackCount += sStartCodeFlag;
	sStartCodeFlag = 0;
	while(!sStartCodeFlag)
	{
		sStartCodeFlag = JudgeNaluStartCodeLen(pStreamBuf[sSendpackCount + sTmpCount]);
		if(sSendpackCount >= sStreamSize - stNalu->startcodeprefix_len)
		{
			sSendpackCount = 0;
			PRINTF("Send last package.\n");
		}
		sTmpCount++;
	}    
	sSendpackCount += sTmpCount;
	stNalu->len = sTmpCount;      
	memcpy(stNalu->buf, &pStreamBuf[sSendpackCount - sTmpCount], stNalu->len);
	stNalu->forbidden_bit = stNalu->buf[0] & 0x80;                    
	stNalu->nal_reference_idc = stNalu->buf[0] & 0x60;                
	stNalu->nal_unit_type = (stNalu->buf[0]) & 0x1f;   
	return SUCCESS_1;                                         
}

/*
*单包，固定起始码 
*输入：pPackAddr、sPackSize、timecode输出：stNalu
*/
void GetOnceNalu_v1(unsigned char *pPackAddr, int sPackSize, unsigned int timecode, NALU_t *stNalu)
{
	memcpy(stNalu->buf, pPackAddr + START_CODE_LEN, sPackSize - START_CODE_LEN);
	stNalu->forbidden_bit = pPackAddr[4] & 0x80;                    
	stNalu->nal_reference_idc = pPackAddr[4] & 0x60;                
	stNalu->nal_unit_type = pPackAddr[4] & 0x1f;  
	stNalu->len = sPackSize - START_CODE_LEN;
	stNalu->timecode = timecode;
	stNalu->ssrc = 10;
}

//回调函数
void GetOneRtpPackage(unsigned char *pPackage, unsigned int sPackagelen)
{
     //处理每个RTP包 例如网络发送   
}

//输入：stNalu  输出：GetOneRtpPackage
int SendRtpData(NALU_t *stNalu,
                  void(*GetOneRtpPackage(unsigned char *pPackage, unsigned int sPackagelen)))
{	
    int size = 0, send_size = 0;
	FU_HEADER *fu_hdr;
	RTP_HEADER  * rtp_hdr = NULL ;
	unsigned char *nalu_payload = NULL; 
	NALU_OR_IND_HEADER *nalu_ind = NULL;
	unsigned char pRtpSendBuf[SEND_MAX_SIZE + 14];
    static  unsigned short package_number = 0;
	
	memset(pRtpSendBuf, 0, SEND_MAX_SIZE + 14);
    rtp_hdr = (RTP_HEADER *)&pRtpSendBuf[0];			 
    rtp_hdr->v		   = 2; 
    rtp_hdr->p		   = 0;
    rtp_hdr->x		   = 0;
    rtp_hdr->cc		   = 0;
    rtp_hdr->pt		   = H264;
    rtp_hdr->ssrc	   = stNalu->ssrc;
	rtp_hdr->timestamp = stNalu->timecode * 90000 / 1000;
    nalu_ind = (NALU_OR_IND_HEADER *)&pRtpSendBuf[12]; 
    nalu_ind->F	       =  stNalu->forbidden_bit >> 7;
    nalu_ind->NRI	   =  stNalu->nal_reference_idc>>5;
	rtp_hdr->m         = 0;
	
	package_number = package_number  >=  0xffff ? 0 : package_number;
	if(stNalu->len < SEND_MAX_SIZE)                                        
	{
        rtp_hdr->seq 	   = package_number++;
        nalu_ind->TYPE     =  stNalu->nal_unit_type; 
		if((0x01 == (stNalu->nal_unit_type & 0x1f)) || (0x05 == (stNalu->nal_unit_type & 0x1f)))
		{
		   rtp_hdr->m		 = 1;
		}
        nalu_payload = &pRtpSendBuf[12]; 	
        memcpy(nalu_payload, stNalu->buf, stNalu->len);
		GetOneRtpPackage(pRtpSendBuf,  stNalu->len + 12);
	}
	else if(stNalu->len >= SEND_MAX_SIZE)                                     
	{
		
	    size  = stNalu->len;
        nalu_ind->TYPE  =  28; 
		while(0 < size)
		{
			rtp_hdr->seq = package_number++;
			
			//在第一个字节跳过nalu头，为了地址长度和总的stNalu->len一致需要在尾包减一
            send_size =  (size <= SEND_MAX_SIZE) ? (size - 1): SEND_MAX_SIZE;
			
			//标记一帧尾包
			if(size <= SEND_MAX_SIZE)
			{
				if((0x01 == (stNalu->nal_unit_type & 0x1f)) ||	(0x05 == (stNalu->nal_unit_type & 0x1f)))
				{
					rtp_hdr->m	 = 1;
				}
			}
			fu_hdr = (FU_HEADER*)&pRtpSendBuf[13]; 
			fu_hdr->S	 = size <= SEND_MAX_SIZE ? 0 : 1;
			fu_hdr->E	 = !fu_hdr->S;
			fu_hdr->R	 = 0;
			fu_hdr->TYPE = stNalu->nal_unit_type;
			nalu_payload = &pRtpSendBuf[14];
			memcpy(nalu_payload, stNalu->buf + stNalu->len - size + 1, send_size);
			size -= SEND_MAX_SIZE;
			GetOneRtpPackage(pRtpSendBuf, send_size + 14);
		}
	}
	return 1;   
}
 

//初始化组播
//输入： IpAddr、GroupPort 输出：sockfd、peeraddr
int InitGroup(int *sockfd, char *IpAddr, int GroupPort, struct sockaddr_in *peeraddr)
{
    unsigned char loop;
    struct ip_mreq mreq;   
 
    /* 创建 socket 用于UDP通讯 */
    sockfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf ("socket creating err in udptalk\n");
        exit (1);
    }
    /* 设置要加入组播的地址 */
    bzero(&mreq, sizeof (struct ip_mreq));
    memset (peeraddr, 0, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, IpAddr, &peeraddr->sin_addr);
    /* 设置组地址 */
    bcopy (&peeraddr->sin_addr.s_addr, &mreq.imr_multiaddr.s_addr, sizeof (struct in_addr));
    /* 设置发送组播消息的源主机的地址信息 */
    mreq.imr_interface.s_addr = htonl (INADDR_ANY);
 
    /* 把本机加入组播地址，即本机网卡作为组播成员，只有加入组才能收到组播消息 */
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP , &mreq,sizeof (struct ip_mreq)) == -1)
    {
        perror ("setsockopt");
        exit (-1);
    }
   loop = 0; 
   if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,sizeof (loop)) == -1)
   {
       printf("IP_MULTICAST_LOOP set fail!\n");
   }
    peeraddr->sin_family = AF_INET;
    peeraddr->sin_port = htons (GroupPort); 
#if 0
    /* 绑定自己的端口和IP信息到socket上 */
    if (bind(sockfd, (struct sockaddr *) peeraddr,sizeof (struct sockaddr_in)) == -1)
    {
        printf ("Bind error\n");
        exit (0);
    }
#endif
    return 0;
}


