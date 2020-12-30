#ifndef BDS_AUDIO_CLIENT_CMD_H__
#define BDS_AUDIO_CLIENT_CMD_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum CMD_KEY {
    CMD_INVALID = -1,
    CMD_WAKEUP_START = 0x100,       //打开唤醒后识别
    CMD_WAKEUP_STOP,                //关闭唤醒后识别
    CMD_ASR_START,                  //打开纯识别, DSP唤醒消息会被屏蔽
    CMD_ASR_STOP,                   //关闭纯识别
    CMD_ENABLE_INSTR,               //打开指令词
    CMD_DISABLE_INSTR,              //关闭指令词
    CMD_ASR_DOT,                    //ASR打点信息下发
    CMD_DYNAMIC_CONFIG
} CMD_KEY_T;

/* ASR打点信息payload格式*/
typedef enum ASR_DOT_TYPE {
    DOT_WP_SUCCESS          =  1,
    DOT_ASR_FIRST_PARTIAL   =  2,
    DOT_ASR_FINAL_RESULT    =  3,
    DOT_ASR_FIRST_TTS       =  4,
    DOT_ASR_FINISH          =  5,
    DOT_ASR_ERR             =  6
} ASR_DOT_TYPE_T;

typedef struct ASR_DOT {
    ASR_DOT_TYPE_T  dot_type;
    char            dot_info[256];
} ASR_DOT_T;
/*
    dot info格式可参考下面的json定义
    1. 唤醒成功
    {\"tagName\":\"DOT_WP_SUCCESS\",\"content\":\"wpWords:小T小T\"}
    2. 首包中间结果
    {\"tagName\":\"DOT_ASR_FIRST_PARTIAL\",\"content\":\"asr_sn:xxxx,sn:xxxx\"}
    3. 最终结果
    {\"tagName\":\"DOT_ASR_FIRST_PARTIAL\",\"content\":\"asr_result:刘德华的电影，asr_sn:xxxx,sn:xxxx\"}
    4. tts首包
    {\"tagName\":\"DOT_ASR_FIRST_TTS\",\"content\":\"tts_text:一起来听 tfboys，大梦想家，asr_sn:xxxx,tts_sn:xxxx\"}
*/

typedef struct AUDIO_CLIENT_CMD {
    CMD_KEY_T cmd_key;
    void *cmd_payload;
    unsigned int payload_length;
} AUDIO_CLIENT_CMD_T;

#if defined(__cplusplus)
}
#endif

#endif // BDS_AUDIO_CLIENT_CMD_H__
