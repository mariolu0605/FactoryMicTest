#ifndef _BD_MIC_DETECT_API_H
#define _BD_MIC_DETECT_API_H
#include<stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_FLAG_MIC_UNUSUAL          0x01
#define ERROR_FLAG_MIC_INCONSIST        0x02
#define ERROR_FLAG_MIC_NONLINEAR        0x04
#define ERROR_FLAG_MIC_ALL_ZEROS        0x08
#define ERROR_FLAG_MIC_OVER_LIMIT       0x10

#define ERROR_FLAG_REF_UNUSUAL          0x20
#define ERROR_FLAG_REF_OVER_LIMIT       0x40
#define ERROR_FLAG_REF_ALL_ZEROS        0x80

typedef enum DETET_TYPE {
    INVALID_DET             = -1,
    SPEAKER_DET             = 1,	//喇叭测试
    INCONSISTENT_DET        = 2,    //一致性检测
    WHOLE_PERF_DET          = 3,    //非线性、一致性等检测
    GAS_TIGHTNESS_DET       = 4     //气密性检测，返回平均能量值
} DETET_TYPE_T;

typedef struct MIC_DETECT_PARAM {
    int         sample_rate;
    int         mic_num;
    int         ref_num;
    float       thld[3];
    int         detect_flag;
} MIC_DETECT_PARAM_T;

struct DETECT_RESULT {
    int         detect_status;
    float       energy_mean;
};

void *bd_mic_detect_init(MIC_DETECT_PARAM_T *param);
int bd_mic_detect_process(void *handle, short *micdata, short *refdata, \
                int last_block_flag, DETECT_RESULT *result);
int bd_mic_detect_uinit(void *handle);
int mic_verify_reset(void *handle);
int bd_get_start_time(FILE *fp_mic, int *npoint);

#ifdef __cplusplus
}
#endif
#endif
