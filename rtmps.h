/********************************************************************  
filename:   RTMPStream.h 
created:    2013-04-3 
author:     firehood  
purpose:    发送H264视频到RTMP Server，使用libRtmp库 
*********************************************************************/   
#include <librtmp/rtmp.h>  
//#include <rtmp_sys.h>  
#include <librtmp/amf.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
  
#define FILEBUFSIZE (1024 * 1024 * 10)       //  10M  
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

#if 0
// NALU单元  
typedef struct _NaluUnit  
{  
    int type;  
    int size;  
    unsigned char *data;  
}NaluUnit;  
#endif
typedef struct _RTMPMetadata  
{  
    // video, must be h264 type  
    unsigned int    nWidth;  
    unsigned int    nHeight;  
    unsigned int    nFrameRate;     // fps  
    unsigned int    nVideoDataRate; // bps  
    unsigned int    nSpsLen;  
    unsigned char   Sps[1024];  
    unsigned int    nPpsLen;  
    unsigned char   Pps[1024];  
  
    // audio, must be aac type  
    bool            bHasAudio;  
    unsigned int    nAudioSampleRate;  
    unsigned int    nAudioSampleSize;  
    unsigned int    nAudioChannels;  
    char            pAudioSpecCfg;  
    unsigned int    nAudioSpecCfgLen;  
  
} RTMPMetadata,*LPRTMPMetadata;  

  
extern void CRTMPStream_init(void);
extern void CRTMPStream_exit(void);

/****************** add by cxb *******************/
extern void CRTMPStream_DeleteStream();
extern int CRTMPStream_IsConnected();
/*************** end by cxb *********************/

// 连接到RTMP Server  
extern bool Connect(const char* url);  
// 断开连接  
extern void Close();  
// 发送MetaData  
extern bool SendMetadata(LPRTMPMetadata lpMetaData);  
// 发送H264数据帧  
extern bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp);  
// 发送AACSpec
extern bool SendAACSpec();
// 发送AAC数据帧
extern bool SendAACPacket(unsigned char* data,unsigned int size,unsigned int nTimeStamp );

// 发送H264文件  
//bool SendH264File(const char *pFileName);  

// 送缓存中读取一个NALU包  
//bool ReadOneNaluFromBuf(NaluUnit &nalu);  
// 发送数据  
//int SendPacket(unsigned int nPacketType,unsigned char *data,unsigned int size,unsigned int nTimestamp);  

//RTMP* m_pRtmp = NULL;  
unsigned char* m_pFileBuf;  
unsigned int  m_nFileBufSize;  
unsigned int  m_nCurPos;  
