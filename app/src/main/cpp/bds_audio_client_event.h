#ifndef BDS_AUDIO_CLIENT_EVENT_H__
#define BDS_AUDIO_CLIENT_EVENT_H__

#if defined(__cplusplus)
extern "C" {
#endif

/* 唤醒和指令词事件payload */
typedef struct WAKEUP_EVENT {
    int             id;                     //唤醒或者指令词唤醒
    char            wakeup_words[32];       //唤醒词或者指令词
    int             location;               //声源方位角
    int             volume;                 //唤醒音量
    unsigned int	wak_beg;                //唤醒词起点
    unsigned int    wak_end;                //唤醒词尾点
    unsigned int    audio_beg;              //识别音频起点
    int             wakeup_zone;            //唤醒音区（RESERVED）
    unsigned int    decoder_param_len;      
    char            *decoder_param;         //decoder_param字段，json字符串
} WAKEUP_EVENT_T;

/* VAD起尾点事件payload */
typedef struct VAD_EVENT {
    unsigned int sequence;    //起尾点水印
} VAD_EVENT_T;

/* DSP状态事件payload */
typedef struct DSP_EVENT {
	unsigned int err_code;   //DSP错误码
	char info[64];     
} DSP_EVENT_T;

/* 音量信息payload */
typedef struct VOLUME_EVENT {
    float   volume;             //音量db
} VOLUME_EVENT_T;

typedef enum EVENT_CODE {
    EVENT_RECORDER_START = 0x100,  //录音机打开成功
    EVENT_RECORDER_STOP  = 0x101,  //录音机关闭成功
    EVENT_WAKEUP,                  //唤醒&指令词触发
    EVENT_ASR_DATA,                //ASR数据回调，RESERVED
    EVENT_VAD_BEGIN,               //VAD起点
    EVENT_VAD_END,                 //VAD尾点
    EVENT_DSP_INFO,                //DSP状态
    EVENT_VOLUME_INFO,             //音量信息
    EVENT_RECORDER_ERROR,          //录音机错误
    EVENT_RECORDER_FIRST_PKG,      //首包数据回调
    EVENT_MAX           = 0x1000
} EVENT_CODE_T;

typedef struct AudioClientEvent {
    EVENT_CODE_T    event_code;
    void            *event_data;
    unsigned int    data_length;
} AudioClientEvent_t;

typedef int (*AudioClientCallback)(AudioClientEvent_t *event, void *arg);

#if defined(__cplusplus)
}
#endif
#endif //BDS_AUDIO_CLIENT_EVENT_H__