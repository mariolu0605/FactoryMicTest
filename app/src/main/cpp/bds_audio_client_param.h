#ifndef BDS_AUDIO_CLIENT_PARAM_H__
#define BDS_AUDIO_CLIENT_PARAM_H__

#if defined(__cplusplus)
extern "C" {
#endif

/* 参数 */
typedef struct AUDIO_CFG_PARAM {
    unsigned int    work_mode;      /* 0：正常模式, 1：工厂模式 */
    unsigned int    param_len;
    char            *param;       /* json字符串 */
} AUDIO_CFG_PARAM_T;

typedef enum {
    NORMAL_MODE     = 0,
    FACTORY_MODE    = 1
} WORK_MODE_T;

#if defined(__cplusplus)
}
#endif

#endif // BDS_AUDIO_CLIENT_PARAM_H__