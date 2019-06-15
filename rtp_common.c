#include "rtp_common.h"

static int  TnitRtp()
{
	if (NULL == (rtp_arg.pRtpFile = fopen(H264_FILE, "rb")))
	{
		PRINTF("Open h264 file is failed.\n");
		return FALSE_0;
	}
	if (NULL == (rtp_arg.stNalu = (NALU_t*)calloc (1, sizeof(NALU_t))))
	{
		PRINTF("nalu calloc is failed.\n");
		return FALSE_0;
	}
	
	rtp_arg.stNalu->max_size = INPUT_FILE_MAX_SIZE; 			//Assign buffer size 
	if (NULL == (rtp_arg.stNalu->buf = (char*)calloc (INPUT_FILE_MAX_SIZE, sizeof (char))))
	{
		free (rtp_arg.stNalu);
		printf ("nalu->buf calloc is failed.\n");
		return FALSE_0;
	}
	return SUCCESS_1;
}


static int InitWinsock()
{
	int sRet = -1;
	WORD Version=MAKEWORD(2,2);
	WSADATA WsaData;
	sRet = WSAStartup(Version,&WsaData);	      //Start up WSA	  
	if(0 != sRet)
	{
	    return FALSE_0;
	}
	else
	{
		if(2 != LOBYTE(WsaData.wVersion) || 2 != HIBYTE(WsaData.wHighVersion))
		{
			WSACleanup();
			return FALSE_0;
		}
	}
	return SUCCESS_1;
}


static void RtpFree()
{
	if (rtp_arg.stNalu)
	{
		if (rtp_arg.stNalu->buf)
		{
			free(rtp_arg.stNalu->buf);
			rtp_arg.stNalu->buf=NULL;
		}
		free(rtp_arg.stNalu);
	}
}


static int CheckStartCode(unsigned char *Buf, int sNum)
{
	int i = -1;
	for(i = 0;i < sNum - 1;i++)
	{
		Buf[0] += Buf[i+1];
	}
	return Buf[0];
}

static int GetOnceNalu()
{
	int pos = 0;                  //一个nal到下一个nal 数据移动的指针
	int rewind = 0;               //判断 前缀所占字节数 3或 4
	unsigned char *Buf = NULL;
    int sStartCodeFlag = 0;
	
	if ((Buf = (unsigned char*)calloc (rtp_arg.stNalu->max_size , sizeof(char))) == NULL) 
	{
		PRINTF ("calloc is failed.\n");
		return FALSE_0;
	}

	rtp_arg.stNalu->startcodeprefix_len = 3;      //初始化前缀位三个字节
	if (3 != fread (Buf, 1, 3, rtp_arg.pRtpFile))//从文件读取三个字节到buf
	{
		free(Buf);
		PRINTF("read h264 head file is failed.\n");
		return 0;
	}
	sStartCodeFlag = CheckStartCode(Buf, 3);       //Check whether Buf is 0x000001
	if(1 != sStartCodeFlag) 
	{
		//If Buf is not 0x000001,then read one more byte
		if(1 != fread(Buf+3, 1, 1, rtp_arg.pRtpFile))
		{
			PRINTF("fread is failed.\n");
			free(Buf);
			return 0;
		}
		sStartCodeFlag = CheckStartCode (Buf, 4);   //Check whether Buf is 0x00000001
		if (sStartCodeFlag != 1)                 //If not the return -1
		{ 
			free(Buf);
			PRINTF("Not a RTP head.\n");
			return FALSE_0;
		}
		else 
		{
			//If Buf is 0x00000001,set the prefix length to 4 bytes
			pos = 4;
			rtp_arg.stNalu->startcodeprefix_len = 4;
		}
	} 
	else
	{
		//If Buf is 0x000001,set the prefix length to 3 bytes
		pos = 3;
		rtp_arg.stNalu->startcodeprefix_len = 3;
	}
	//寻找下一个字符符号位， 即 寻找一个nal 从一个0000001 到下一个00000001
	sStartCodeFlag = 0;
	while (!sStartCodeFlag)//找下一个nal头
	{
		if (feof (rtp_arg.pRtpFile))   //如果到了文件结尾
		{
			rtp_arg.stNalu->len = (pos-1) - rtp_arg.stNalu->startcodeprefix_len;  //从0 开始 
			memcpy (rtp_arg.stNalu->buf, &Buf[rtp_arg.stNalu->startcodeprefix_len], rtp_arg.stNalu->len);     
			rtp_arg.stNalu->forbidden_bit = rtp_arg.stNalu->buf[0] & 0x80;      // 1 bit--10000000
			rtp_arg.stNalu->nal_reference_idc = rtp_arg.stNalu->buf[0] & 0x60;  // 2 bit--01100000
			rtp_arg.stNalu->nal_unit_type = (rtp_arg.stNalu->buf[0]) & 0x1f;    // 5 bit--00011111
			free(Buf);
			return pos-1;
		}
		Buf[pos++] = fgetc(rtp_arg.pRtpFile);                       //Read one char to the Buffer 一个字节一个字节从文件向后找
		sStartCodeFlag = CheckStartCode(&Buf[pos-4], 4);		        //Check whether Buf is 0x00000001 
		if(sStartCodeFlag != 1)
		{
			sStartCodeFlag = CheckStartCode(&Buf[pos-3], 3);            //Check whether Buf is 0x000001
		}
	}

	rewind = (sStartCodeFlag == 1)? -4 : -3;

	if (0 != fseek(rtp_arg.pRtpFile, rewind, SEEK_CUR))			    //将文件内部指针移动到 nal 的末尾
	{
		free(Buf);
		PRINTF("fseek is failed.\n");
		return FALSE_0;
	}

	rtp_arg.stNalu->len = (pos + rewind) -  rtp_arg.stNalu->startcodeprefix_len;       //设置包含nal 头的数据长度
	memcpy(rtp_arg.stNalu->buf, &Buf[rtp_arg.stNalu->startcodeprefix_len], rtp_arg.stNalu->len);//拷贝一个nal 数据到数组中
	rtp_arg.stNalu->forbidden_bit = rtp_arg.stNalu->buf[0] & 0x80;                     //1 bit  设置nal 头
	rtp_arg.stNalu->nal_reference_idc = rtp_arg.stNalu->buf[0] & 0x60;                 // 2 bit
	rtp_arg.stNalu->nal_unit_type = (rtp_arg.stNalu->buf[0]) & 0x1f;                   // 5 bit
	free(Buf);
	return (pos + rewind);                                         //Return the length of bytes from between one NALU and the next NALU
}

static void FillNaluHead()
{
	NALU_HEADER * nalu_hdr;
	nalu_hdr =(NALU_HEADER*)&rtp_arg.sendbuf[12];  
	nalu_hdr->F    = rtp_arg.stNalu->forbidden_bit >> 7;
	nalu_hdr->NRI  = rtp_arg.stNalu->nal_reference_idc >> 5; 
	nalu_hdr->TYPE = rtp_arg.stNalu->nal_unit_type;
}

static void FillFuIndicat()
{
	FU_INDICATOR *fu_ind;
	fu_ind =(FU_INDICATOR*)&sendbuf[12]; 
	fu_ind->F=rtp_arg.stNalu->forbidden_bit >> 7;
	fu_ind->NRI=rtp_arg.stNalu->nal_reference_idc>>5;
	fu_ind->TYPE=28;
}

static void FillFuHead(int S, int E, int R)
{
	FU_HEADER *fu_hdr;
	fu_hdr =(FU_HEADER*)&sendbuf[13]; 
	fu_hdr->S=S;
	fu_hdr->E=E;
	fu_hdr->R=R;
	fu_hdr->TYPE=rtp_arg.stNalu->nal_unit_type;
}

static void RtpFree()
{
    FreeNALU(rtp_arg.stNalu);             //释放nal 资源
	fclose(rtp_arg.pRtpFile);
	WSACleanup();
	closesocket(socket_cli);
}

static int GetRtpData()
{	
	RTP_HEADER  * rtp_hdr = NULL ;
	NALU_HEADER * nalu_hdr = NULL;
	char* nalu_payload = NULL; 
	int timestamp = 0;
	int  bytes = 0;           //一次发送的数据
	printf("NALU--- forbidden_bit		 :	   %d\n", rtp_arg.stNalu->forbidden_bit);
	printf("NALU--- nal_reference_idc	 :	   %d\n", rtp_arg.stNalu->nal_reference_idc);
	printf("NALU--- Type				 :	   %d\n", rtp_arg.stNalu->nal_unit_type);
	printf("NALU--- startcodeprefix_len  :	   %d\n", rtp_arg.stNalu->startcodeprefix_len);
	printf("NALU--- len 				 :	   %d\n", rtp_arg.stNalu->len);
	printf("NALU--- max_size			 :	   %d\n", rtp_arg.stNalu->max_size);
	
	memset(sendbuf,0,MAX_DATA_ASIZE);
	rtp_hdr =(RTP_HEADER*)&sendbuf[0];			  
	rtp_hdr->payloadtype	 = H264;							//Payload type
	rtp_hdr->version		 = 2;								//Payload version
	rtp_hdr->marker 		 = 0;								//Marker sign
	rtp_hdr->ssrc			 = htonl(rtp_arg.frame_number++);			//帧数	
    rtp_arg.sSendFlag = 0;
	if (rtp_arg.stNalu->len < 1400)                                          //打单包
	{
		rtp_hdr->marker = 1;
		rtp_hdr->seq_no = htons(rtp_arg.pocket_number ++); 
        rtp_hdr->timestamp = htonl(timestamp);
		FillNaluHead();
        nalu_payload = &sendbuf[13];               
        memcpy(nalu_payload,rtp_arg.stNalu->buf+1,rtp_arg.stNalu->len-1); 
        timestamp = timestamp + 6000;
        bytes = rtp_arg.stNalu->len + 12 ;						 
        Sleep(40);
	    rtp_arg.sSendFlag = 1;
        rtp_arg.total_sent += bytes;
		//printf("单包：Total bytes : %d\n",bytes);
	}
	else if (rtp_arg.stNalu->len >= 1400)                                      //打分片包
	{
		int k=0;
		int l=0;
		int t = 0;                                                //分片包 片号
		k = rtp_arg.stNalu->len / 1400;                                        //分片数目
		l = rtp_arg.stNalu->len % 1400;                                        //最后一个包的长度
		timestamp = timestamp + 6000;
		rtp_hdr->timestamp = htonl(timestamp);
		rtp_arg.sSendFlag = 0;
		while(t <= k)
		{
		    rtp_arg.sSendFlag = 0;
			rtp_hdr->seq_no = htons(rtp_arg.pocket_number ++);            //包号
			if(!t)                                                //分片打包第一个分片
			{
				rtp_hdr->marker = 0;
				FillFuIndicat();
				FillFuHead(1, 0, 0);
				nalu_payload=&sendbuf[14];
				memcpy(nalu_payload,rtp_arg.stNalu->buf+1,1400);        
				bytes=1400 + 14;						       
				Sleep(40);
				rtp_arg.sSendFlag = 1;
				t++;                                        //分片包的第几包
				rtp_arg.total_sent += bytes;
				//printf("分片打包第一个分片：Total bytes : %d\n",bytes);
			}
			else if(k == t)                                 //发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
			{
				rtp_hdr->marker = 1;
				FillFuIndicat();
				FillFuHead(0, 1, 0);
				nalu_payload=&sendbuf[14];
				memcpy(nalu_payload,rtp_arg.stNalu->buf + t*1400+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
				bytes = l-1+14;		                       //获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
				Sleep(40);
				rtp_arg.sSendFlag = 0;
				t++;
				rtp_arg.total_sent += bytes;
				//printf("分片打包最后一个分片 ：Total bytes : %d\n",bytes);
			}
			else if(t < k && 0 != t)                      //分片打包不是最后一个包也不是第一个包               
			{
				rtp_hdr->marker=0;
				FillFuIndicat();
                FillFuHead(0, 0, 0);
				nalu_payload=&sendbuf[14];                //同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload,rtp_arg.stNalu->buf + t * 1400 + 1,1400);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
				bytes=1400+14;	
				Sleep(40);
				rtp_arg.sSendFlag = 1;
				t++;
				rtp_arg.total_sent += bytes;
				//printf("分片打包中间分片：Total bytes : %d\n",bytes);
			}
		}
	}
	

}

RTP_ARG_LIST rtp_arg = 
{
    .pRtpFile = NULL,
    .stNalu = NULL,
    .frame_number = 0,
	.pocket_number = 0,
	.total_sent = 0,
	.sendbuf = {0};
	.sSendFlag = 0,
}
///////////////////////////////
/*
sInitWinsock();
TnitRtp();
//创建线程 如果rtp_arg.sSendFlag为1发送数据
while(!feof(rtp_arg.pRtpFile))  //如果未到文件结尾
{
	usleep(1);//降低cpu占用
	GetOnceNalu(); 
	GetRtpData()；				
}
RtpFree();
*/
/////////////////////////////////
