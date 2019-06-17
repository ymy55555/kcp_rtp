#include "rtp_common.h"

int  TnitRtpSend()
{
	if (NULL == (rtp_send_arg.pRtpFile = fopen(H264_SEND_FILE, "rb")))
	{
		PRINTF("Open h264 file is failed.\n");
		return FALSE_0;
	}
	if (NULL == (rtp_send_arg.stNalu = (NALU_t*)calloc (1, sizeof(NALU_t))))
	{
		PRINTF("nalu calloc is failed.\n");
		return FALSE_0;
	}
	
	rtp_send_arg.stNalu->max_size = INPUT_FILE_MAX_SIZE; 			//Assign buffer size 
	if (NULL == (rtp_send_arg.stNalu->buf = (char*)calloc (INPUT_FILE_MAX_SIZE, sizeof (char))))
	{
		free (rtp_send_arg.stNalu);
		PRINTF ("nalu->buf calloc is failed.\n");
		return FALSE_0;
	}
	return SUCCESS_1;
}


static int  TnitRtpRecv()
{
	if (NULL == (rtp_recv_arg.pRtpFile = fopen(H264_RECV_FILE, "wb")))
	{
		PRINTF("Open h264 file is failed.\n");
		return FALSE_0;
	}
	if (NULL == (rtp_recv_arg.stNalu = (NALU_t*)calloc (1, sizeof(NALU_t))))
	{
		PRINTF("nalu calloc is failed.\n");
		return FALSE_0;
	}
	rtp_recv_arg.stNalu->max_size = OUTPUT_FILE_MAX_SIZE; 			//Assign buffer size 
	if (NULL == (rtp_recv_arg.stNalu->buf = (char*)calloc (INPUT_FILE_MAX_SIZE, sizeof (char))))
	{
		free (rtp_recv_arg.stNalu);
		PRINTF ("nalu->buf calloc is failed.\n");
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


static int CheckStartCode(unsigned char *Buf, int sNum)
{
	int i = -1;
	for(i = 0;i < sNum - 1;i++)
	{
		Buf[0] += Buf[i+1];
	}
	return Buf[0];
}


static void FillNaluHead()
{
	NALU_HEADER * nalu_hdr;
	nalu_hdr =(NALU_HEADER*)&rtp_send_arg.sendbuf[12];  
	nalu_hdr->F    = rtp_send_arg.stNalu->forbidden_bit >> 7;
	nalu_hdr->NRI  = rtp_send_arg.stNalu->nal_reference_idc >> 5; 
	nalu_hdr->TYPE = rtp_send_arg.stNalu->nal_unit_type;
}

static void FillFuIndicat()
{
	FU_INDICATOR *fu_ind;
	fu_ind =(FU_INDICATOR*)&rtp_send_arg.sendbuf[12]; 
	fu_ind->F=rtp_send_arg.stNalu->forbidden_bit >> 7;
	fu_ind->NRI=rtp_send_arg.stNalu->nal_reference_idc>>5;
	fu_ind->TYPE=28;
}

static void FillFuHead(int S, int E, int R)
{
	FU_HEADER *fu_hdr;
	fu_hdr =(FU_HEADER*)&rtp_send_arg.sendbuf[13]; 
	fu_hdr->S=S;
	fu_hdr->E=E;
	fu_hdr->R=R;
	fu_hdr->TYPE=rtp_send_arg.stNalu->nal_unit_type;
}


static void FreeNalu(NALU_t *Nalu)
{
	if (Nalu)
	{
		if (Nalu->buf)
		{
			free(Nalu->buf);
			Nalu->buf=NULL;
		}
		free(Nalu);
	}
}

static void RtpSendFree()
{
    FreeNalu(rtp_send_arg.stNalu);             //释放nal 资源
	fclose(rtp_send_arg.pRtpFile);
	WSACleanup();
}

static void RtpRecvFree()
{
    FreeNalu(rtp_recv_arg.stNalu);             //释放nal 资源
	free(rtp_recv_arg.pRtpPack->payload);
	free(rtp_recv_arg.pRtpPack);
	fclose(rtp_recv_arg.pRtpFile);
	WSACleanup();
}


int  rtp_unpackage(char *bufIn, int len)
{
    unsigned char recvbuf[1500];
	RTP_HEADER *rtp_hdr = NULL;
	NALU_HEADER *nalu_hdr = NULL;
	FU_INDICATOR	*fu_ind = NULL;
	FU_HEADER		*fu_hdr= NULL;
	memcpy(recvbuf,bufIn, len);          //复制rtp包 
	PRINTF("包长度+ rtp头：   = %d\n",len);

	rtp_recv_arg.pRtpPack = (RTPpacket_t*)&recvbuf[0];
	rtp_hdr =(RTP_HEADER*)&recvbuf[0]; 
	rtp_recv_arg.pRtpPack->v  = rtp_hdr->version;
	rtp_recv_arg.pRtpPack->p  = rtp_hdr->padding;
	rtp_recv_arg.pRtpPack->x  = rtp_hdr->extension;
	rtp_recv_arg.pRtpPack->cc = rtp_hdr->csrc_len;
	rtp_recv_arg.pRtpPack->m = rtp_hdr->marker;
	rtp_recv_arg.pRtpPack->pt = rtp_hdr->payloadtype;
	rtp_recv_arg.pRtpPack->seq = rtp_hdr->seq_no;
	rtp_recv_arg.pRtpPack->timestamp = rtp_hdr->timestamp;
	rtp_recv_arg.pRtpPack->ssrc = rtp_hdr->ssrc;
	
	PRINTF("版本号 : %d\n",rtp_hdr->version);
	PRINTF("标志位 : %d\n",rtp_hdr->marker);
	PRINTF("负载类型:%d\n",rtp_hdr->payloadtype);
	PRINTF("包号   : %d \n",rtp_hdr->seq_no);
	PRINTF("时间戳 : %d\n",rtp_hdr->timestamp);
	PRINTF("帧号   : %d\n",rtp_hdr->ssrc);
	nalu_hdr =(NALU_HEADER*)&recvbuf[12];                        //网络传输过来的字节序 ，当存入内存还是和文档描述的相反，只要匹配网络字节序和文档描述即可传输正确。
	PRINTF("forbidden_zero_bit: %d\n",nalu_hdr->F);              //网络传输中的方式为：F->NRI->TYPE.. 内存中存储方式为 TYPE->NRI->F (和nal头匹配)。
	rtp_recv_arg.stNalu->forbidden_bit= nalu_hdr->F << 7;                          //内存中的字节序。
	PRINTF("nal_reference_idc:  %d\n",nalu_hdr->NRI);
	rtp_recv_arg.stNalu->nal_reference_idc = nalu_hdr->NRI << 5;                      
	PRINTF("nal 负载类型:       %d\n",nalu_hdr->TYPE);
	rtp_recv_arg.stNalu->nal_unit_type = nalu_hdr->TYPE;

	if ( nalu_hdr->TYPE  == 0)
	{
		PRINTF("这个包有错误，0无定义\n");
	}
	else if ( nalu_hdr->TYPE >0 &&  nalu_hdr->TYPE < 24)  //单包
	{
		PRINTF("当前包为单包\n");
		putc(0x00, rtp_recv_arg.pRtpFile);
		putc(0x00, rtp_recv_arg.pRtpFile);
		putc(0x00, rtp_recv_arg.pRtpFile);
		putc(0x01, rtp_recv_arg.pRtpFile);
		rtp_recv_arg.total_bytes +=4;
		memcpy(rtp_recv_arg.pRtpPack->payload,&recvbuf[13],len-13);	
		rtp_recv_arg.pRtpPack->paylen = len-13;
	 	fwrite(nalu_hdr,1,1,rtp_recv_arg.pRtpFile);
		rtp_recv_arg.total_bytes += 1;
		rtp_recv_arg.fwrite_number = fwrite(rtp_recv_arg.pRtpPack->payload,1,rtp_recv_arg.pRtpPack->paylen,rtp_recv_arg.pRtpFile);
		rtp_recv_arg.total_bytes = rtp_recv_arg.pRtpPack->paylen;
		PRINTF("包长度 + nal= %d\n",rtp_recv_arg.total_bytes);
	}
	else if ( nalu_hdr->TYPE == 24)                    //STAP-A   单一时间的组合包
	{
		PRINTF("当前包为STAP-A\n");
	}
	else if ( nalu_hdr->TYPE == 25)                    //STAP-B   单一时间的组合包
	{
		PRINTF("当前包为STAP-B\n");
	}
	else if (nalu_hdr->TYPE == 26)                     //MTAP16   多个时间的组合包
	{
		PRINTF("当前包为MTAP16\n");
	}
	else if ( nalu_hdr->TYPE == 27)                    //MTAP24   多个时间的组合包
	{
		PRINTF("当前包为MTAP24\n");
	}
    else if ( nalu_hdr->TYPE == 28)                    //FU-A分片包，解码顺序和传输顺序相同
	{
		fu_ind=(FU_INDICATOR*)&recvbuf[12];
		PRINTF("FU_INDICATOR->F     :%d\n",fu_ind->F);
		rtp_recv_arg.stNalu->forbidden_bit = fu_ind->F << 7;
		PRINTF("FU_INDICATOR->NRI   :%d\n",fu_ind->NRI);
		rtp_recv_arg.stNalu->nal_reference_idc = fu_ind->NRI << 5;                      
		PRINTF("FU_INDICATOR->TYPE  :%d\n",fu_ind->TYPE);
		rtp_recv_arg.stNalu->nal_unit_type = fu_ind->TYPE;

		fu_hdr=(FU_HEADER*)&recvbuf[13];
		PRINTF("FU_HEADER->S        :%d\n",fu_hdr->S);
		PRINTF("FU_HEADER->E        :%d\n",fu_hdr->E);
		PRINTF("FU_HEADER->R        :%d\n",fu_hdr->R);
		PRINTF("FU_HEADER->TYPE     :%d\n",fu_hdr->TYPE);
		rtp_recv_arg.stNalu->nal_unit_type = fu_hdr->TYPE;               //应用的是FU_HEADER的TYPE

		if (rtp_hdr->marker == 1)                      //分片包最后一个包
		{
			PRINTF("当前包为FU-A分片包最后一个包\n");
			memcpy(rtp_recv_arg.pRtpPack->payload,&recvbuf[14],len - 14);
			rtp_recv_arg.pRtpPack->paylen = len - 14;
			rtp_recv_arg.fwrite_number = fwrite(rtp_recv_arg.pRtpPack->payload,1,rtp_recv_arg.pRtpPack->paylen,rtp_recv_arg.pRtpFile);
			rtp_recv_arg.total_bytes = rtp_recv_arg.pRtpPack->paylen;
			PRINTF("包长度 + FU = %d\n",rtp_recv_arg.total_bytes);	
		}
		else if (rtp_hdr->marker == 0)                 //分片包 但不是最后一个包
		{
			if (fu_hdr->S == 1)                        //分片的第一个包
			{
				unsigned char F;
				unsigned char NRI;
				unsigned char TYPE;
				unsigned char fui;

				unsigned char S;
				unsigned char E;
				unsigned char R;
				unsigned char Type;
				unsigned char fuh;
				
				PRINTF("当前包为FU-A分片包第一个包\n");
				putc(0x00, rtp_recv_arg.pRtpFile);
				putc(0x00, rtp_recv_arg.pRtpFile);
				putc(0x00, rtp_recv_arg.pRtpFile);
				putc(0x01, rtp_recv_arg.pRtpFile);
				rtp_recv_arg.total_bytes += 4;
                
				F = fu_ind->F << 7;
				NRI = fu_ind->NRI << 5;
				TYPE = fu_hdr->TYPE;                                            //应用的是FU_HEADER的TYPE
				//nh = rtp_recv_arg.stNalu->forbidden_bit|rtp_recv_arg.stNalu->nal_reference_idc|rtp_recv_arg.stNalu->nal_unit_type;  //二进制文件也是按 大字节序存储
				fui = F | NRI | TYPE;

				putc(fui, rtp_recv_arg.pRtpFile);

				S = fu_hdr.S << 7;
				E = fu_hdr.E << 6;
				R = fu_hdr.R << 5;
				Type  = fu_hdr.TYPE;
				fuh = S | E | R | Type;
				putc(fuh, rtp_recv_arg.pRtpFile);
				
				rtp_recv_arg.total_bytes +=1;
				memcpy(rtp_recv_arg.pRtpPack->payload,&recvbuf[14],len - 14);
				rtp_recv_arg.pRtpPack->paylen = len - 14;
				rtp_recv_arg.fwrite_number = fwrite(rtp_recv_arg.pRtpPack->payload,1,rtp_recv_arg.pRtpPack->paylen,rtp_recv_arg.pRtpFile);
				rtp_recv_arg.total_bytes = rtp_recv_arg.pRtpPack->paylen;
				PRINTF("包长度 + FU_First = %d\n",rtp_recv_arg.total_bytes);	
			}
			else                                      //如果不是第一个包
			{
				PRINTF("当前包为FU-A分片包\n");
				memcpy(rtp_recv_arg.pRtpPack->payload,&recvbuf[14],len - 14);
				rtp_recv_arg.pRtpPack->paylen= len - 14;
				rtp_recv_arg.fwrite_number = fwrite(rtp_recv_arg.pRtpPack->payload,1,rtp_recv_arg.pRtpPack->paylen,rtp_recv_arg.pRtpFile);
				rtp_recv_arg.total_bytes = rtp_recv_arg.pRtpPack->paylen;
				PRINTF("包长度 + FU = %d\n",rtp_recv_arg.total_bytes);	
			}	
		}
	}
	else if ( nalu_hdr->TYPE == 29)                //FU-B分片包，解码顺序和传输顺序相同
	{
		if (rtp_hdr->marker == 1)                  //分片包最后一个包
		{
			PRINTF("当前包为FU-B分片包最后一个包\n");

		}
		else if (rtp_hdr->marker == 0)             //分片包 但不是最后一个包
		{
			PRINTF("当前包为FU-B分片包\n");
		}
	}
	else
	{
		PRINTF("这个包有错误，30-31 没有定义\n");
	}
	rtp_recv_arg.total_recved += rtp_recv_arg.total_bytes;
	PRINTF("rtp_recv_arg.total_recved = %d\n",rtp_recv_arg.total_recved);
    return 1;
}

static int GetOnceNalu()
{
	int pos = 0;                  //一个nal到下一个nal 数据移动的指针
	int rewind = 0;               //判断 前缀所占字节数 3或 4
	unsigned char *Buf = NULL;
    int sStartCodeFlag = 0;
	
	if ((Buf = (unsigned char*)calloc (rtp_send_arg.stNalu->max_size , sizeof(char))) == NULL) 
	{
		PRINTF ("calloc is failed.\n");
		return FALSE_0;
	}

	rtp_send_arg.stNalu->startcodeprefix_len = 3;      //初始化前缀位三个字节
	if (3 != fread (Buf, 1, 3, rtp_send_arg.pRtpFile))//从文件读取三个字节到buf
	{
		free(Buf);
		PRINTF("read h264 head file is failed.\n");
		return 0;
	}
	sStartCodeFlag = CheckStartCode(Buf, 3);       //Check whether Buf is 0x000001
	if(1 != sStartCodeFlag) 
	{
		//If Buf is not 0x000001,then read one more byte
		if(1 != fread(Buf+3, 1, 1, rtp_send_arg.pRtpFile))
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
			rtp_send_arg.stNalu->startcodeprefix_len = 4;
		}
	} 
	else
	{
		//If Buf is 0x000001,set the prefix length to 3 bytes
		pos = 3;
		rtp_send_arg.stNalu->startcodeprefix_len = 3;
	}
	//寻找下一个字符符号位， 即 寻找一个nal 从一个0000001 到下一个00000001
	sStartCodeFlag = 0;
	while (!sStartCodeFlag)//找下一个nal头
	{
		if (feof (rtp_send_arg.pRtpFile))   //如果到了文件结尾
		{
			rtp_send_arg.stNalu->len = (pos-1) - rtp_send_arg.stNalu->startcodeprefix_len;  //从0 开始 
			memcpy (rtp_send_arg.stNalu->buf, &Buf[rtp_send_arg.stNalu->startcodeprefix_len], rtp_send_arg.stNalu->len);     
			rtp_send_arg.stNalu->forbidden_bit = rtp_send_arg.stNalu->buf[0] & 0x80;      // 1 bit--10000000
			rtp_send_arg.stNalu->nal_reference_idc = rtp_send_arg.stNalu->buf[0] & 0x60;  // 2 bit--01100000
			rtp_send_arg.stNalu->nal_unit_type = (rtp_send_arg.stNalu->buf[0]) & 0x1f;    // 5 bit--00011111
			free(Buf);
			return pos-1;
		}
		Buf[pos++] = fgetc(rtp_send_arg.pRtpFile);                       //Read one char to the Buffer 一个字节一个字节从文件向后找
		sStartCodeFlag = CheckStartCode(&Buf[pos-4], 4);		        //Check whether Buf is 0x00000001 
		if(sStartCodeFlag != 1)
		{
			sStartCodeFlag = CheckStartCode(&Buf[pos-3], 3);            //Check whether Buf is 0x000001
		}
	}

	rewind = (sStartCodeFlag == 1)? -4 : -3;

	if (0 != fseek(rtp_send_arg.pRtpFile, rewind, SEEK_CUR))			    //将文件内部指针移动到 nal 的末尾
	{
		free(Buf);
		PRINTF("fseek is failed.\n");
		return FALSE_0;
	}

	rtp_send_arg.stNalu->len = (pos + rewind) -  rtp_send_arg.stNalu->startcodeprefix_len;       //设置包含nal 头的数据长度
	memcpy(rtp_send_arg.stNalu->buf, &Buf[rtp_send_arg.stNalu->startcodeprefix_len], rtp_send_arg.stNalu->len);//拷贝一个nal 数据到数组中
	rtp_send_arg.stNalu->forbidden_bit = rtp_send_arg.stNalu->buf[0] & 0x80;                     //1 bit  设置nal 头
	rtp_send_arg.stNalu->nal_reference_idc = rtp_send_arg.stNalu->buf[0] & 0x60;                 // 2 bit
	rtp_send_arg.stNalu->nal_unit_type = (rtp_send_arg.stNalu->buf[0]) & 0x1f;                   // 5 bit
	free(Buf);
	return (pos + rewind);                                         //Return the length of bytes from between one NALU and the next NALU
}

static int GetRtpData()
{	
	RTP_HEADER  * rtp_hdr = NULL ;
	NALU_HEADER * nalu_hdr = NULL;
	char* nalu_payload = NULL; 
	int timestamp = 0;
	int  bytes = 0;           //一次发送的数据
	PRINTF("NALU--- forbidden_bit		 :	   %d\n", rtp_send_arg.stNalu->forbidden_bit);
	PRINTF("NALU--- nal_reference_idc	 :	   %d\n", rtp_send_arg.stNalu->nal_reference_idc);
	PRINTF("NALU--- Type				 :	   %d\n", rtp_send_arg.stNalu->nal_unit_type);
	PRINTF("NALU--- startcodeprefix_len  :	   %d\n", rtp_send_arg.stNalu->startcodeprefix_len);
	PRINTF("NALU--- len 				 :	   %d\n", rtp_send_arg.stNalu->len);
	PRINTF("NALU--- max_size			 :	   %d\n", rtp_send_arg.stNalu->max_size);
	
	memset(rtp_send_arg.sendbuf,0,MAX_DATA_SIZE);
	rtp_hdr =(RTP_HEADER*)&rtp_send_arg.sendbuf[0];			  
	rtp_hdr->payloadtype	 = H264;							//Payload type
	rtp_hdr->version		 = 2;								//Payload version
	rtp_hdr->marker 		 = 0;								//Marker sign
	rtp_hdr->ssrc			 = htonl(rtp_send_arg.frame_number++);			//帧数	
    rtp_send_arg.sSendFlag = 0;
	if (rtp_send_arg.stNalu->len < 1400)                                          //打单包
	{
		rtp_hdr->marker = 1;
		rtp_hdr->seq_no = htons(rtp_send_arg.pocket_number ++); 
        rtp_hdr->timestamp = htonl(timestamp);
		FillNaluHead();
        nalu_payload = &rtp_send_arg.sendbuf[13];               
        memcpy(nalu_payload,rtp_send_arg.stNalu->buf+1,rtp_send_arg.stNalu->len-1); 
        timestamp = timestamp + 6000;
        bytes = rtp_send_arg.stNalu->len + 12 ;						 
        Sleep(40);
	    rtp_send_arg.sSendFlag = 1;
        rtp_send_arg.total_sent += bytes;
		//PRINTF("单包：Total bytes : %d\n",bytes);
	}
	else if (rtp_send_arg.stNalu->len >= 1400)                                      //打分片包
	{
		int k=0;
		int l=0;
		int t = 0;                                                //分片包 片号
		k = rtp_send_arg.stNalu->len / 1400;                                        //分片数目
		l = rtp_send_arg.stNalu->len % 1400;                                        //最后一个包的长度
		timestamp = timestamp + 6000;
		rtp_hdr->timestamp = htonl(timestamp);
		rtp_send_arg.sSendFlag = 0;
		while(t <= k)
		{
		    rtp_send_arg.sSendFlag = 0;
			rtp_hdr->seq_no = htons(rtp_send_arg.pocket_number ++);            //包号
			if(!t)                                                //分片打包第一个分片
			{
				rtp_hdr->marker = 0;
				FillFuIndicat();
				FillFuHead(1, 0, 0);//nalu分片开始
				nalu_payload=&rtp_send_arg.sendbuf[14];
				memcpy(nalu_payload,rtp_send_arg.stNalu->buf+1,1400);        
				bytes=1400 + 14;						       
				Sleep(40);
				rtp_send_arg.sSendFlag = 1;
				t++;                                        //分片包的第几包
				rtp_send_arg.total_sent += bytes;
				//PRINTF("分片打包第一个分片：Total bytes : %d\n",bytes);
			}
			else if(k == t)                                 //发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
			{
				rtp_hdr->marker = 1;
				FillFuIndicat();
				FillFuHead(0, 1, 0);//nalu分片结束
				nalu_payload=&rtp_send_arg.sendbuf[14];
				memcpy(nalu_payload,rtp_send_arg.stNalu->buf + t*1400+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
				bytes = l-1+14;		                       //获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
				Sleep(40);
				rtp_send_arg.sSendFlag = 0;
				t++;
				rtp_send_arg.total_sent += bytes;
				//PRINTF("分片打包最后一个分片 ：Total bytes : %d\n",bytes);
			}
			else if(t < k && 0 != t)                      //分片打包不是最后一个包也不是第一个包               
			{
				rtp_hdr->marker=0;
				FillFuIndicat();
                FillFuHead(0, 0, 0);//nalu分片介于开始和结束之间位置
				nalu_payload=&rtp_send_arg.sendbuf[14];                //同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload,rtp_send_arg.stNalu->buf + t * 1400 + 1,1400);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
				bytes=1400+14;	
				Sleep(40);
				rtp_send_arg.sSendFlag = 1;
				t++;
				rtp_send_arg.total_sent += bytes;
				//PRINTF("分片打包中间分片：Total bytes : %d\n",bytes);
			}
		}
	}
	

}

RTP_SEND_ARG rtp_send_arg = 
{
    .pRtpFile = NULL,
    .stNalu = NULL,
    .frame_number = 0,
	.pocket_number = 0,
	.total_sent = 0,
	.sendbuf = {0};
	.sSendFlag = 0,
}

RTP_RECV_ARG rtp_recv_arg = 
{
   .pRtpFile = NULL,
   .stNalu = NULL,
   .total_bytes = -1,				  //当前包传出的数据
   .total_recved = -1,		  //一共传输的数据
   .fwrite_number = -1,		  //存入文件的数据长度
   .pRtpPack = NULL,
}

///////////////send操所示例////////////////
/*
sInitWinsock();
TnitRtpSend();
//创建线程 如果rtp_arg.sSendFlag为1发送数据
while(!feof(rtp_send_arg.pRtpFile))  //如果未到文件结尾
{
	usleep(1);//降低cpu占用
	GetOnceNalu(); 
	GetRtpData()；				
}
RtpSendFree();
*/
/////////////////////////////////
