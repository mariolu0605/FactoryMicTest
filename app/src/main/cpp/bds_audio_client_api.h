#ifndef BDS_AUDIO_CLIENT_API_H__
#define BDS_AUDIO_CLIENT_API_H__

#include "bds_audio_client_event.h"
#include "bds_audio_client_cmd.h"
#include "bds_audio_client_param.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define API_PUBLIC __attribute__((visibility("default")))
typedef void* AUDIO_HANDLE_T;

//实时流数据类型
typedef enum REQUEST_TYPE {     
    INVALID = -1,
    ASR     = 1,    /* 连续实时交织的ASR数据 16K-4CH */
    COMM    = 2,    /* 连续实时交织的通讯数据 16K-2CH */
    ORIG    = 3     /* 连续实时交织的原始数据 16K-1MIC+1REF 本项目暂不提供 */
} REQ_TYPE_T;

/**
 * bds audio client create routine, create audio client
 *
 * @param in - param, bds audio client configure params
 * @return - 0, means sucessfull
 *           - !0 means fail
 */
API_PUBLIC AUDIO_HANDLE_T bds_audio_create(AUDIO_CFG_PARAM_T *param);

/**
 * bds audio client destroy routine.
 * 
 * @param in - handle, bds audio client handle
 * @return - 0, means sucessfull 
 *           - !0 means fail
 */
API_PUBLIC int bds_audio_destroy(AUDIO_HANDLE_T handle);

/**
 * bds audio client start routine, open data link and recorder
 * 
 * @param in - handle, bds audio client handle
 * @return - 0, means sucessfull 
 *           - !0 means fail
 */
API_PUBLIC int bds_audio_start(AUDIO_HANDLE_T handle);

/**
 * bds audio client stop routine, close data link and recorder
 * 
 * @param in - handle, bds audio client handle
 * @return - 0, means sucessfull 
 *           - !0 means fail
 */
API_PUBLIC int bds_audio_stop(AUDIO_HANDLE_T handle);

/**
 * bds audio client ioctl routine
 * 
 * @param in - handle, bds audio client handle
 * @param in - cmd, iotcl cmd struct
 * @return - 0, means sucessfull 
 *           - !0 means fail
 */
API_PUBLIC int bds_audio_ioctl(AUDIO_HANDLE_T handle, AUDIO_CLIENT_CMD_T *cmd);

/**
 * bds audio client callback register routine.
 * 
 * @param in - handle, bds audio client handle
 * @param in - cb, event callback
 * @return - 0, means sucessfull 
 *           - !0 means fail
 */
API_PUBLIC int bds_audio_set_event_listener(AUDIO_HANDLE_T handle, AudioClientCallback cb);

/**
 * bds audio client data read routine.
 * 
 * @param in - handle, bds audio client handle
 * @param in - type, 1：asr data stream, 2: comm data stream, 3: orignal data stream
 * @param in - data, data buffer
 * @param in - len, data length 
 * @return - 0, means sucessfull 
 *           - !0 means fail
 */
API_PUBLIC int bds_audio_read(AUDIO_HANDLE_T handle, REQ_TYPE_T type, void *data, unsigned int len);

/**
 * bds audio client version get routine.
 * 
 * @param in - handle, bds audio client handle
 * @return - versoin string of bds audio client
 */
API_PUBLIC const char* bds_audio_versoin(AUDIO_HANDLE_T handle);

/**
 * bds audio client waterprint get routine, 
 * get tag from 32bytes in the head of 8ms data frame
 * 
 * @param in - pcm_data, single channel pcm data frame(8ms) buffer
 * @param in - len, extract waterprint from [len] bytes pcm data, set 32 here
 * @param out - seq_num, waterprint value
 * @return - 0, means sucessfull
 *          - !0 means fail
 */
API_PUBLIC int bds_get_tag_from_pcm(short* pcm_data, int len, unsigned short* seq_num);

#if defined(__cplusplus)
}
#endif

#endif //BDS_AUDIO_CLIENT_API_H__
