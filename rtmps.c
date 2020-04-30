/********************************************************************  
filename:   RTMPStream.cpp 
created:    2013-04-3 
author:     firehood  
purpose:    发送H264视频到RTMP Server，使用libRtmp库 
*********************************************************************/   
#include "RTMPStream.h"  
//#include "SpsDecode.h"  
#include <signal.h>

  
enum  
{  
    FLV_CODECID_H264 = 7,  
};  
  
int InitSockets()    
{    
#ifdef WIN32    
    WORD version;    
    WSADATA wsaData;    
    version = MAKEWORD(1, 1);    
    return (WSAStartup(version, &wsaData) == 0);    
#else    
    return TRUE;    
#endif    
}    
  
inline void CleanupSockets()    
{    
#ifdef WIN32    
    WSACleanup();    
#endif    
}    
  
char * put_byte( char *output, uint8_t nVal )    
{    
    output[0] = nVal;    
    return output+1;    
}    
char * put_be16(char *output, uint16_t nVal )    
{    
    output[1] = nVal & 0xff;    
    output[0] = nVal >> 8;    
    return output+2;    
}    
char * put_be24(char *output,uint32_t nVal )    
{    
    output[2] = nVal & 0xff;    
    output[1] = nVal >> 8;    
    output[0] = nVal >> 16;    
    return output+3;    
}    
char * put_be32(char *output, uint32_t nVal )    
{    
    output[3] = nVal & 0xff;    
    output[2] = nVal >> 8;    
    output[1] = nVal >> 16;    
    output[0] = nVal >> 24;    
    return output+4;    
}    
char *  put_be64( char *output, uint64_t nVal )    
{    
    output=put_be32( output, nVal >> 32 );    
    output=put_be32( output, nVal );    
    return output;    
}    
char * put_amf_string( char *c, const char *str )    
{    
    uint16_t len = strlen( str );    
    c=put_be16( c, len );    
    memcpy(c,str,len);    
    return c+len;    
}    
char * put_amf_double( char *c, double d )    
{    
    *c++ = AMF_NUMBER;  /* type: Number */    
    {    
        unsigned char *ci, *co;    
        ci = (unsigned char *)&d;    
        co = (unsigned char *)c;    
        co[0] = ci[7];    
        co[1] = ci[6];    
        co[2] = ci[5];    
        co[3] = ci[4];    
        co[4] = ci[3];    
        co[5] = ci[2];    
        co[6] = ci[1];    
        co[7] = ci[0];    
    }    
    return c+8;    
}  
RTMP* m_pRtmp = NULL;
void CRTMPStream_init(void)
{  
    if(m_pRtmp == NULL)
    {
    	m_pRtmp = RTMP_Alloc();    
    	RTMP_Init(m_pRtmp);    
    }

	(void)signal(SIGPIPE, SIG_IGN);		/* ignore SIGPIPE signal */
}  
  
void CRTMPStream_exit(void)  
{  
	if(m_pRtmp)  
    {  
        RTMP_Close(m_pRtmp);  
        RTMP_Free(m_pRtmp);  
        m_pRtmp = NULL;  
    }  
}  
  
bool Connect(const char* url)  
{  
    if(RTMP_SetupURL(m_pRtmp, (char*)url) == FALSE)  
    {  
        return FALSE;  
    }  
    RTMP_EnableWrite(m_pRtmp);  
    if(RTMP_Connect(m_pRtmp, NULL) == FALSE)  
    {  
        return FALSE;  
    }  
    if(RTMP_ConnectStream(m_pRtmp,0) == FALSE)  
    {  
        return FALSE;  
    }  
    return TRUE;  
}  
  
void Close()  
{  
    if(m_pRtmp)  
    {  
        RTMP_Close(m_pRtmp);  
        RTMP_Free(m_pRtmp);  
        m_pRtmp = NULL;  
    }  
}  

/****************** add by cxb *******************/
void CRTMPStream_DeleteStream()
{
	RTMP_DeleteStream(m_pRtmp);
	printf("\nalready delete stream-lib...\n");
}

int CRTMPStream_IsConnected()
{
	if(!RTMP_IsConnected(m_pRtmp))
	{
		printf("\nRTMPStream is not connect-lib...\n");
		return 0;
	}
	/*printf("\nRTMPStream is connect-lib...\n");*/
	return 1;
}
/*************** end by cxb *********************/

int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp)  
{  
    if(m_pRtmp == NULL)  
    {  
        return FALSE;  
    }  
  
    RTMPPacket packet;  
    RTMPPacket_Reset(&packet);  
    RTMPPacket_Alloc(&packet,size);  
  
    packet.m_packetType = nPacketType;  
    packet.m_nChannel = 0x04;    
    packet.m_headerType = RTMP_PACKET_SIZE_LARGE;    
    packet.m_nTimeStamp = nTimestamp;    
    packet.m_nInfoField2 = m_pRtmp->m_stream_id;  
    packet.m_nBodySize = size;  
    memcpy(packet.m_body,data,size);  
  
    int nRet = RTMP_SendPacket(m_pRtmp,&packet,0);  
  
    RTMPPacket_Free(&packet);  
  
    return nRet;  
}  
  
bool SendMetadata(LPRTMPMetadata lpMetaData)  
{  
	char tmp_data = 0xAC;	//1010 11 0 0
	char tmp_data1 = 0x10;	//00010 0000 0000 0 0 0
	char tmp_data2 = 0x00;	
	
    if(lpMetaData == NULL)  
    {  
        return false;  
    }  
    char body[1024] = {0};
      
    char * p = (char *)body;    
    p = put_byte(p, AMF_STRING );  
    p = put_amf_string(p , "@setDataFrame" );  
  
    p = put_byte( p, AMF_STRING );  
    p = put_amf_string( p, "onMetaData" );  
  
    p = put_byte(p, AMF_OBJECT );    
    p = put_amf_string( p, "copyright" );    
    p = put_byte(p, AMF_STRING );    
    p = put_amf_string( p, "firehood" );    
  
    p =put_amf_string( p, "width");  
    p =put_amf_double( p, lpMetaData->nWidth);  
  
    p =put_amf_string( p, "height");  
    p =put_amf_double( p, lpMetaData->nHeight);  
  
    p =put_amf_string( p, "framerate" );  
    p =put_amf_double( p, lpMetaData->nFrameRate);   
  
    p =put_amf_string( p, "videocodecid" );  
    p =put_amf_double( p, FLV_CODECID_H264 );  
  
    p =put_amf_string( p, "" );  
    p =put_byte( p, AMF_OBJECT_END  );  
  
    int index = p-body;  
  
    SendPacket(RTMP_PACKET_TYPE_INFO,(unsigned char*)body,p-body,0);  
  
    int i = 0;  
    body[i++] = 0x17; // 1:keyframe  7:AVC  
    body[i++] = 0x00; // AVC sequence header  
  
    body[i++] = 0x00;  
    body[i++] = 0x00;  
    body[i++] = 0x00; // fill in 0;  
  
    // AVCDecoderConfigurationRecord.  
    body[i++] = 0x01; // configurationVersion  
    body[i++] = lpMetaData->Sps[1]; // AVCProfileIndication  
    body[i++] = lpMetaData->Sps[2]; // profile_compatibility  
    body[i++] = lpMetaData->Sps[3]; // AVCLevelIndication   
    body[i++] = 0xff; // lengthSizeMinusOne    
  
    // sps nums  
    body[i++] = 0xE1; //&0x1f  
    // sps data length  
    body[i++] = (lpMetaData->nSpsLen>>8) & 0xff;  
    body[i++] = lpMetaData->nSpsLen & 0xff;  
    // sps data  
    memcpy(&body[i],lpMetaData->Sps,lpMetaData->nSpsLen);  
    i= i+lpMetaData->nSpsLen;  
  
    // pps nums  
    body[i++] = 0x01; //&0x1f  
    // pps data length   
    body[i++] = (lpMetaData->nPpsLen>>8) & 0xff;  
    body[i++] = lpMetaData->nPpsLen & 0xff;  
    // sps data  
    memcpy(&body[i],lpMetaData->Pps,lpMetaData->nPpsLen);  
    i= i+lpMetaData->nPpsLen;  
  
    SendPacket(RTMP_PACKET_TYPE_VIDEO,(unsigned char*)body,i,0);  
/***********************AAC**************************/
	if(lpMetaData->bHasAudio == 1)
	{
		if(lpMetaData->nAudioSampleSize == 16)
		{
			tmp_data |= 0x02;//16-bit
		}else if(lpMetaData->nAudioSampleSize == 8)
		{
			tmp_data |= 0x00;//8-bit
		}else //default 16-bit
		{
			tmp_data |= 0x02;
		}

		
		if(lpMetaData->nAudioChannels == 0)
		{
			tmp_data |= 0x00;//mono sound
			tmp_data2 |= 0x08;
		}else if(lpMetaData->nAudioChannels == 1)
		{
			tmp_data |= 0x01;//stereo sound
			tmp_data2 |= 0x10;
		}else //default mono sound
		{
			tmp_data |= 0x00;
			tmp_data2 |= 0x08;
		}
        tmp_data |= 0x01;// aac always is 1
        
		switch(lpMetaData->nAudioSampleRate)
		{
			case 96000:
				tmp_data1 |= ((0x00 >> 1) & 0x7);
				tmp_data2 |= ((0x00 & 0x01) << 7);
			break;
			case 88200:
				tmp_data1 |= ((0x01 >> 1) & 0x7);
				tmp_data2 |= ((0x01 & 0x01) << 7);
			break;
			case 64000:
				tmp_data1 |= ((0x02 >> 1) & 0x7);
				tmp_data2 |= ((0x02 & 0x01) << 7);
			break;
			case 48000:
				tmp_data1 |= ((0x03 >> 1) & 0x7);
				tmp_data2 |= ((0x03 & 0x01) << 7);
			break;
			case 44100:
				tmp_data1 |= ((0x04 >> 1) & 0x7);
				tmp_data2 |= ((0x04 & 0x01) << 7);
			break;
			case 32000:
				tmp_data1 |= ((0x05 >> 1) & 0x7);
				tmp_data2 |= ((0x05 & 0x01) << 7);
			break;
			case 24000:
				tmp_data1 |= ((0x06 >> 1) & 0x7);
				tmp_data2 |= ((0x06 & 0x01) << 7);
			break;
			case 22050:
				tmp_data1 |= ((0x07 >> 1) & 0x7);
				tmp_data2 |= ((0x07 & 0x01) << 7);
			break;
			case 16000:
				tmp_data1 |= ((0x08 >> 1) & 0x7);
				tmp_data2 |= ((0x08 & 0x01) << 7);
			break;
			case 12000:
				tmp_data1 |= ((0x09 >> 1) & 0x7);
				tmp_data2 |= ((0x09 & 0x01) << 7);
			break;
			case 11025:
				tmp_data1 |= ((0x0a >> 1) & 0x7);
				tmp_data2 |= ((0x0a & 0x01) << 7);
			break;
			case 8000:
				tmp_data1 |= ((0x0b >> 1) & 0x7);
				tmp_data2 |= ((0x0b & 0x01) << 7);
			break;
			default://48kHz
				tmp_data1 |= ((0x04 >> 1) & 0x7);
				tmp_data2 |= ((0x04 & 0x01) << 7);			
			break;

		}
		i = 0;  
		//body[i++] = 0xAE;
	    body[i++] = tmp_data; //A:AAC head E:44kHz sam,16-bit samples,mono sound
	    body[i++] = 0x00; //AAC sequence header	
	    body[i++] = tmp_data1;
	    body[i++] = tmp_data2;
	    printf("AAC SPEC:%02X,00,%02X,%02X\n", tmp_data, tmp_data1, tmp_data2); 
	    SendPacket(RTMP_PACKET_TYPE_AUDIO,(unsigned char*)body,i,0);
	}
	return 0;
}  
  
bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp)  
{  
    if(data == NULL && size<11)  
    {  
        return false;  
    }  
  	unsigned char *body = (unsigned char *)malloc(sizeof(unsigned char) * (size+9));
    //unsigned char *body = new unsigned char[size+9];  
  
    int i = 0;  
    if(bIsKeyFrame)  
    {  
        body[i++] = 0x17;// 1:Iframe  7:AVC  
    }  
    else  
    {  
        body[i++] = 0x27;// 2:Pframe  7:AVC  
    }  
    body[i++] = 0x01;// AVC NALU  
    body[i++] = 0x00;  
    body[i++] = 0x00;  
    body[i++] = 0x00;  
  
    // NALU size  
    body[i++] = size>>24;  
    body[i++] = size>>16;  
    body[i++] = size>>8;  
    body[i++] = size&0xff;;  
  
    // NALU data  
    memcpy(&body[i],data,size);  
  
    bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,body,i+size,nTimeStamp);  
  
//    delete[] body;  
  	free(body);
    return bRet;  
}  

bool Send_H264SpsPps_Packet(unsigned char *sps, unsigned int sps_len, unsigned char *pps, unsigned int pps_len, unsigned int nTimeStamp)  
{ 
    int head_len = 0;
    char *head_buf = NULL;

    if (!sps || !pps)
    {
        //printf("sps pps is null \n");
        return 1;
    }

    if (!sps_len)
    {
        //printf("sps len is 0 \n");
        return 1;
    }
    
    head_len = sps_len + pps_len + 16;

  	head_buf = (unsigned char *)malloc(head_len);
    if (NULL == head_buf)
    {
        printf("malloc fail \n");
        return 1;
    }
      
    //int i = 0;  

    head_buf[0] = 0x17;// 1:Iframe  7:AVC  
    head_buf[1] = 0x00;  
    head_buf[2] = 0x00;  
    head_buf[3] = 0x00;  
    head_buf[4] = 0x00;  
    head_buf[5] = 0x00; 
    head_buf[6] = sps[1];  
    head_buf[7] = sps[2];  
    head_buf[8] = sps[3];  
  
    head_buf[9] = 0x03;  
    head_buf[10] = 0xe1;  
    put_be16(head_buf+11, (uint16_t)sps_len);
    memcpy(head_buf+13, sps, sps_len);

    head_buf[13+sps_len] = 0x1;  
    put_be16(head_buf+13+sps_len+1, (uint16_t)pps_len);
    memcpy(head_buf+13+1+2+sps_len, pps, pps_len);
    
    bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,head_buf,head_len,nTimeStamp);  
  
  	free(head_buf);
    return bRet;  
}

#if 0
/*AAC*/
bool SendAACSpec()
{
	RTMPPacket * packet;
    unsigned char * body;
    int len =2;
    //char spec_buf[4]={0x15,0x90}; 8000HZ : 0X1590; 44100HZ: 0x1210;   
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+len+2);
    memset(packet,0,RTMP_HEAD_SIZE);
 
    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    body = (unsigned char *)packet->m_body;
 
    /*AF 00 + AAC RAW data*/
    body[0] = 0xAF;
    body[1] = 0x00;
    //memcpy(&body[2],spec_buf,len); /*spec_buf是AAC sequence header数据*/
	body[2] = 0x11;//0x12;
	body[3] = 0x90;//0x10;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = len+2;
    packet->m_nChannel = 0x04;
    packet->m_nTimeStamp = 0;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nInfoField2 = m_pRtmp->m_stream_id;
 
    /*调用发送接口*/
    RTMP_SendPacket(m_pRtmp,packet,0/*TRUE*/);
 	free(packet);
    return TRUE;
}
#endif

bool SendAACPacket(unsigned char* data,unsigned int size,unsigned int nTimeStamp )
{
 
	 if (size > 0) 
	 {
		 RTMPPacket * packet;
		 unsigned char * body;
 
		 packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size+2);
		 memset(packet,0,RTMP_HEAD_SIZE);
 
		 packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
		 body = (unsigned char *)packet->m_body;
 
		 /*AF 01 + AAC RAW data*/
		 body[0] = 0xAF;
		 body[1] = 0x01;//0x01;
		 memcpy(&body[2],data,size);
 
		 ///SendPacket(RTMP_PACKET_TYPE_AUDIO,body,2+size,nTimeStamp);
 
		 packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
		 packet->m_nBodySize = size+2;
		 packet->m_nChannel = 0x04;
		 packet->m_nTimeStamp = nTimeStamp;
		 packet->m_hasAbsTimestamp = 0;
		 packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
		 packet->m_nInfoField2 = m_pRtmp->m_stream_id;
 
		 /*调用发送接口*/
		 RTMP_SendPacket(m_pRtmp,packet,0 /*TRUE*/);
		 free(packet);
	 }
	 return true;
}


 #if 0
bool CRTMPStream::SendH264File(const char *pFileName)  
{  
    if(pFileName == NULL)  
    {  
        return FALSE;  
    }  
    FILE *fp = fopen(pFileName, "rb");    
    if(!fp)    
    {    
        printf("ERROR:open file %s failed!",pFileName);  
    }    
    fseek(fp, 0, SEEK_SET);  
    m_nFileBufSize = fread(m_pFileBuf, sizeof(unsigned char), FILEBUFSIZE, fp);  
    if(m_nFileBufSize >= FILEBUFSIZE)  
    {  
        printf("warning : File size is larger than BUFSIZE\n");  
    }  
    fclose(fp);    
  
    RTMPMetadata metaData;  
    memset(&metaData,0,sizeof(RTMPMetadata));  
  
    NaluUnit naluUnit;  
    // 读取SPS帧  
    ReadOneNaluFromBuf(naluUnit);  
    metaData.nSpsLen = naluUnit.size;  
	//printf("sps len is %u\n", metaData.nSpsLen);
    memcpy(metaData.Sps,naluUnit.data,naluUnit.size);  
  
    // 读取PPS帧  
    ReadOneNaluFromBuf(naluUnit);  
    metaData.nPpsLen = naluUnit.size;  
	//printf("pps len is %u\n", metaData.nSpsLen);
    memcpy(metaData.Pps,naluUnit.data,naluUnit.size);  
  
    // 解码SPS,获取视频图像宽、高信息  
    int width = 0,height = 0;  
    h264_decode_sps(metaData.Sps,metaData.nSpsLen,width,height);  
    metaData.nWidth = width;  
    metaData.nHeight = height;  
    metaData.nFrameRate = 25;  
     
    // 发送MetaData  
    SendMetadata(&metaData);  
  
    unsigned int tick = 0;  
    while(ReadOneNaluFromBuf(naluUnit))  
    {  
        bool bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;  
        // 发送H264数据帧  
        SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick);  
        msleep(40);  
        tick +=40;  
    }  
  
    return TRUE;  
}  
 
bool CRTMPStream::ReadOneNaluFromBuf(NaluUnit &nalu)  
{  
    int i = m_nCurPos;  
    while(i<m_nFileBufSize-4)  
    {  
        if(m_pFileBuf[i++] == 0x00 &&  
            m_pFileBuf[i++] == 0x00 &&  
            m_pFileBuf[i++] == 0x00 &&  
            m_pFileBuf[i++] == 0x01  
            )  
        {  
            int pos = i;  
            while (pos<m_nFileBufSize-4)  
            {  
                if(m_pFileBuf[pos++] == 0x00 &&  
                    m_pFileBuf[pos++] == 0x00 &&  
                    m_pFileBuf[pos++] == 0x00 &&  
                    m_pFileBuf[pos++] == 0x01  
                    )  
                {  
                    break;  
                }  
            }  
            //if(pos == nBufferSize)  
            //{  
                //nalu.size = pos-i;    
            //}  
            //else  
            //{  
                nalu.size = (pos-4)-i;  
            //}  
            nalu.type = m_pFileBuf[i]&0x1f;  
            nalu.data = &m_pFileBuf[i];  
  
            m_nCurPos = pos-4;  
            return TRUE;  
        }  
    }  
    return FALSE;  
}
 #endif


//====================================================================
#include "RTMPStream.h"
#define RTMP_CONFIG_URL "rtmp://192.168.5.82:1935/live/xiaoyang"
 
 static pthread_t s_RTMP_pthread[5];
 static int RTMP_send_flag = FALSE;
 typedef struct
 {
	 unsigned char* DataBuf;
	 int NumBuf[11];
 }STREAM_BUF_T;
 
 //0 :包数	1~4：每个包大小		  5~8：每个包地址的偏移量 9:所有包大小总和
void copy_hisi_video_stream(STREAM_BUF_T* stStreamTmp, VENC_STREAM_S* stStream)
{
#define _ADDR(i) (stStream->pstPack[i].pu8Addr + stStream->pstPack[i].u32Offset)
#define _LEN(i)  (stStream->pstPack[i].u32Len - stStream->pstPack[i].u32Offset)
#define TOTAL_SIZE  (_LEN(0) + _LEN(1) + _LEN(2) + _LEN(3))

	 int i, size = 0;
	 stStreamTmp->NumBuf[0] = stStream->u32PackCount;
	 if (1 == stStream->u32PackCount && _LEN(0) > 1920*1080)
	 {
		 stStreamTmp->DataBuf = (unsigned char*)realloc(stStreamTmp->DataBuf, _LEN(0) + 1);
	 }
	 else if(4 == stStream->u32PackCount && TOTAL_SIZE > 1920*1080)
	 {
		 stStreamTmp->DataBuf = (unsigned char*)realloc(stStreamTmp->DataBuf, TOTAL_SIZE + 1);
	 }
	 for (i = 0; i < stStream->u32PackCount; i++)
	 {
		 stStreamTmp->NumBuf[i + 5] = size;
		 memcpy(stStreamTmp->DataBuf + size, _ADDR(i), _LEN(i));
		 size += _LEN(i);
		 stStreamTmp->NumBuf[i + 1] = _LEN(i);
	 }
	 stStreamTmp->NumBuf[i + 5] = size;
}
 
void RTMP_send_h264(STREAM_BUF_T* stStreamTmp, VENC_STREAM_S* stStream)
{

#define PACK_ADDR(i)  (stStreamTmp->DataBuf + stStreamTmp->NumBuf[i + 5])
#define PACK_LEN(i)   (stStreamTmp->NumBuf[i + 1])
#define THROW_LEN 4
	 RTMPMetadata MetaData;
	 static int nFirstFlag = 1;
	 unsigned int nTimeStamp;	 
	 static unsigned int nTimeStampTmp;

	 static int nKeyFrameFlag = 1;
	 MetaData.bHasAudio = 0;
	 MetaData.nFrameRate = 30;
	 MetaData.nHeight = 1080;
	 MetaData.nWidth = 1920;
	 MetaData.nVideoDataRate = 4096;

	 nTimeStamp = stStream->pstPack[0].u64PTS / 1000;
	 if (nKeyFrameFlag && 4 != stStreamTmp->NumBuf[0])
	 {
		 HI_MPI_VENC_ResetChn(1);
		 HI_MPI_VENC_StartRecvPic(1);
		 HI_MPI_VENC_RequestIDR(1, TRUE);
		 nTimeStampTmp = stStream->pstPack[0].u64PTS / 1000;
		 return;
	 }else
	 {
		 nKeyFrameFlag = 0;
	 }

	 nTimeStamp -= nTimeStampTmp;
	 if (1 == stStreamTmp->NumBuf[0])
	 {
		 SendH264Packet(PACK_ADDR(0) + THROW_LEN, PACK_LEN(0) - THROW_LEN, 0, nTimeStamp);
	 }
	 else
	 {

		 memcpy(MetaData.Sps, PACK_ADDR(0) + THROW_LEN, PACK_LEN(0) - THROW_LEN);
		 memcpy(MetaData.Pps, PACK_ADDR(1) + THROW_LEN, PACK_LEN(1) - THROW_LEN);
		 MetaData.nSpsLen = PACK_LEN(0) - THROW_LEN;
		 MetaData.nPpsLen = PACK_LEN(1) - THROW_LEN;
		 if (nFirstFlag)
		 {
			 nFirstFlag = 0;
			 SendMetadata(&MetaData);
			 Send_H264SpsPps_Packet(PACK_ADDR(0) + THROW_LEN, PACK_LEN(0) - THROW_LEN, PACK_ADDR(1) + THROW_LEN, PACK_LEN(1) - THROW_LEN, nTimeStamp);
		 }
		 SendH264Packet(PACK_ADDR(3) + THROW_LEN, PACK_LEN(3) - THROW_LEN, 1, nTimeStamp);
	 }
}
 
static void RTMP_thread()
{

	 pthread_detach(pthread_self());
	 while (1)
	 {
#if 1
		 if (Connect(RTMP_CONFIG_URL))
		 {

			 RTMP_send_flag = TRUE;
			 break;
		 }
#endif

	 }
 printf("rtmp is link.\n");
 pthread_exit(0);
}
 
 static void RTMP_init()
 {

	 pthread_create(&s_RTMP_pthread[0], NULL, (void*)RTMP_thread, NULL);
 }
 
 
