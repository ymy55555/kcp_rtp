#define  H264 96                   
#define SEND_MAX_SIZE 1440   //可以设置更大，比如64K，降低CPU占用，需要服务端做好接受处理
#define START_CODE_LEN 4

#define SUCCESS_0   0
#define FALSE_0     0
#define SUCCESS_1   1
#define FALSE_1     1


#define PRINTF(fmt...)   \
    do {\
        printf("[%s]:%d:  ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)

	/*
	* RTP data header
	*/
	typedef struct 
	{
#if 0 //BIG_ENDIA
		 unsigned int v:2;	/* protocol version */
		 unsigned int p:1; /* padding flag */
		 unsigned int x:1; /* header extension flag */
		 unsigned int cc:4; /* CSRC count */
		 unsigned int m:1;	/* marker bit */
		 unsigned int pt:7; /* payload type */
		 unsigned int seq:16; /* sequence number */
#else //SMALL_END
		 unsigned int cc:4; /* CSRC count */
		 unsigned int x:1; /* header extension flag */
		 unsigned int p:1;	/* padding flag */
		 unsigned int v:2;  /* protocol version */
		 unsigned int pt:7; /* payload type */
		 unsigned int m:1;	/* marker bit */
		 unsigned int seq:16; /* sequence number */
#endif
		 HI_U32 timestamp; /* timestamp */
		 HI_U32 ssrc;  /* synchronization source */
	} RTP_HEADER;

typedef struct
{
	unsigned char forbidden_bit;           
	unsigned char nal_reference_idc;       
	unsigned char nal_unit_type;           
	unsigned int startcodeprefix_len;      
	unsigned int len;                   
	unsigned int max_size;                 
	unsigned int lost_packets;             
	unsigned char *buf;
	unsigned int timecode;
	unsigned int ssrc;
} NALU_t;

typedef struct 
{
	unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;        
} NALU_OR_IND_HEADER; // 1 BYTE 

typedef struct 
{
	//byte 0
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;          //分片包第一个包 S = 1 ,其他= 0 。    
} FU_HEADER;   // 1 BYTES 


 typedef struct _rtp_common_arg_
 {

 }RTP_COMMON_ARG;

//外部接口
extern RTP_COMMON_ARG rtp_common_arg;






