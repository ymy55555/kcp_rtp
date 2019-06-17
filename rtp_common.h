#ifndef _RTP_COMMON_H_
#define _RTP_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <conio.h>
//#include <WinSock2.h>

//#pragma comment(lib,"ws2_32.lib")

#define INPUT_FILE_MAX_SIZE 104857600//100M
#define OUTPUT_FILE_MAX_SIZE 104857600//100M
#define H264_SEND_FILE "./sender.264"
#define H264_RECV_FILE "./recv.264"
#define MAX_DATA_SIZE 1400
#define  H264 96                   

#define SUCCESS_0   0
#define FALSE_0     0
#define SUCCESS_1   1
#define FALSE_1     1



#define PRINTF(fmt...)   \
    do {\
        printf("[%s]:%d:  ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)


typedef struct
{
	unsigned char v;               //!< 版本号
	unsigned char p;			   //!< Padding bit, Padding MUST NOT be used
	unsigned char x;			   //!< Extension, MUST be zero
	unsigned char cc;       	   //!< CSRC count, normally 0 in the absence of RTP mixers 		
	unsigned char m;			   //!< Marker bit 如果单包 m = 1 ,分片包的最后一个包 = 1，分片包其他包 m = 0 ,组合包不准确 
	unsigned char pt;			   //!< 7 bits, 负载类型 H264 96 
	unsigned int seq;			   //!< 包号
	unsigned int timestamp;	       //!< timestamp, 27 MHz for H.264 事件戳
	unsigned int ssrc;			   //!< 真号
	unsigned char *  payload;      //!< the payload including payload headers
	unsigned int paylen;		   //!< length of payload in bytes
} RTPpacket_t;

typedef struct 
{
	/*  0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|V=2|P|X|  CC   |M|     PT      |       sequence number         |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                           timestamp                           |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           synchronization source (SSRC) identifier            |
	+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	|            contributing source (CSRC) identifiers             |
	|                             ....                              |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
	//intel 的cpu 是intel为小端字节序（低端存到底地址） 而网络流为大端字节序（高端存到低地址）
	/*intel 的cpu ： 高端->csrc_len:4 -> extension:1-> padding:1 -> version:2 ->低端
	 在内存中存储 ：
	 低->4001（内存地址）version:2
	     4002（内存地址）padding:1
		 4003（内存地址）extension:1
	 高->4004（内存地址）csrc_len:4

     网络传输解析 ： 高端->version:2->padding:1->extension:1->csrc_len:4->低端  (为正确的文档描述格式)

	 存入接收内存 ：
	 低->4001（内存地址）version:2
	     4002（内存地址）padding:1
	     4003（内存地址）extension:1
	 高->4004（内存地址）csrc_len:4
	 本地内存解析 ：高端->csrc_len:4 -> extension:1-> padding:1 -> version:2 ->低端 ，
	 即：
	 unsigned char csrc_len:4;        // expect 0 
	 unsigned char extension:1;       // expect 1
	 unsigned char padding:1;         // expect 0 
	 unsigned char version:2;         // expect 2 
	*/
	/* byte 0 */
	 unsigned char csrc_len:4;        /* expect 0 */
	 unsigned char extension:1;       /* expect 1, see RTP_OP below */
	 unsigned char padding:1;         /* expect 0 */
	 unsigned char version:2;         /* expect 2 */
	/* byte 1 */
	 unsigned char payloadtype:7;     /* RTP_PAYLOAD_RTSP */
	 unsigned char marker:1;          /* expect 1 */
	/* bytes 2,3 */
	 unsigned int seq_no;            
	/* bytes 4-7 */
	 unsigned int timestamp;        
	/* bytes 8-11 */
	 unsigned int ssrc;              /* stream number is used here. */
} RTP_HEADER;


typedef struct
{
	unsigned char forbidden_bit;           //! Should always be FALSE
	unsigned char nal_reference_idc;       //! NALU_PRIORITY_xxxx
	unsigned char nal_unit_type;           //! NALU_TYPE_xxxx  
	unsigned int startcodeprefix_len;      //! 预留
	unsigned int len;                      //! 预留
	unsigned int max_size;                 //! 预留
	unsigned char * buf;                   //! 预留
	unsigned int lost_packets;             //! 预留
} NALU_t;

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*/
typedef struct 
{
	//byte 0
	unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;        
} NALU_HEADER; // 1 BYTE 

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*/
typedef struct 
{
	//byte 0
	unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;              
} FU_INDICATOR; // 1 BYTE 

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+
*/
typedef struct 
{
	//byte 0
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;          //分片包第一个包 S = 1 ,其他= 0 。    
} FU_HEADER;   // 1 BYTES 

typedef struct _rtp_send_arg_
{
     FILE * pRtpFile;
	 NALU_t *stNalu;
	 int frame_number;	   //帧号
	 int pocket_number;   //包号
	 int total_sent;	   //已经发送的总共数据
	 unsigned char sendbuf[MAX_DATA_SIZE];
	 int sSendFlag;
}RTP_SEND_ARG;
typedef struct _rtp_recv_arg_
{
     FILE * pRtpFile;
	 NALU_t *stNalu;
	 int total_bytes;				  //当前包传出的数据
	 static int total_recved;		  //一共传输的数据
	 int fwrite_number;			  //存入文件的数据长度
	 RTPpacket_t *pRtpPack;
}RTP_RECV_ARG;

/***********************对外接口***********/
/* FIXME...
外部操作需要锁
某些统计变量需要定期在特定情况下清零
有些函数形参指针函数内是否释放待考虑
减少全局变量，尽量只留对外有用接口
*/
extern RTP_SEND_ARG rtp_send_arg;
extern RTP_RECV_ARG rtp_recv_arg;
extern int  TnitRtpSend();
extern int  TnitRtpRecv();
extern int InitWinsock();
extern int CheckStartCode(unsigned char *Buf, int sNum);
extern void FillNaluHead();
extern void FillFuIndicat();
extern void FillFuHead(int S, int E, int R);
extern void RtpSendFree(NALU_t *Nalu);
static void RtpRecvFree();
extern int  rtp_unpackage(char *bufIn, int len);
extern int GetOnceNalu();
extern int GetRtpData();

#endif
