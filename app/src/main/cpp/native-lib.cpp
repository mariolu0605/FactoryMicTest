#include <jni.h>
#include <string>
#include <android/log.h>
#include <bitset>

#include <unistd.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#include "bd_mic_detect_api.h"
#include "bds_audio_client_api.h"

extern "C" {
extern int test();
}

#define TAG "MarioLu"
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__);
#define ALOGD(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__);
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);


#define TEST_PCM_FILE       "/data/data/com.mario.factorymictest/audio.pcm"
#define db_to_ratio(x)      powf(10, x / 20.f)
#define DEFAULT_PERIODS     (3)
#define ONE_FRAME_SZ        (1536)      //8ms

#define MIC_8MS_SAMP_CNT  (128)
#define MIC_NUM           (4)
#define REF_NUM           (2)
#define NFRM              (625)

FILE *fd_rec = NULL;

volatile bool forceExit = false;
AUDIO_HANDLE_T client;
jclass jniCls;
jmethodID bdsCallBack;
jstring jmsg;
JNIEnv *vm;
jobject thizz;
JavaVM *jvm = NULL;
char str[300];
char szTmp[ONE_FRAME_SZ] = {0};


extern "C" JNIEXPORT jstring JNICALL
Java_com_mario_factorymictest_jni_FactoryMicTestJNI_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    ALOGE("test() = %d\n", test());
    return env->NewStringUTF(hello.c_str());

}

//唤醒
int audio_wakeup_start(AUDIO_HANDLE_T client) {
    ALOGE("WAKEUP+ASR start\n");
    const char *param = "wakeup start";
    AUDIO_CLIENT_CMD_T cmd = {
            .cmd_key = CMD_WAKEUP_START,
            .cmd_payload = (void *) param,
            .payload_length = strlen(param) + 1
    };
    return bds_audio_ioctl(client, &cmd);
}

//关闭唤醒
int audio_wakeup_stop(AUDIO_HANDLE_T client) {
    ALOGE("WAKEUP+ASR stop\n");
    const char *param = "wakeup stop";
    AUDIO_CLIENT_CMD_T cmd = {
            .cmd_key = CMD_WAKEUP_STOP,
            .cmd_payload = (void *) param,
            .payload_length = strlen(param) + 1
    };
    return bds_audio_ioctl(client, &cmd);
}

int handle_sig(int signo) {
    ALOGE("Received a signal %d\n", signo);
    forceExit = true;
    signal(signo, SIG_DFL);
    return 0;
}

void wakeup_stop() {
    if (client) {
        (void) audio_wakeup_stop(client);
    }
}

void clear() {
    if (client) {
        (void) bds_audio_stop(client);
        (void) bds_audio_destroy(client);
    }
    if (fd_rec) {
        ALOGE("fd_rec close")
        fclose(fd_rec);
    }
}

void bds_callback(char *msg) {
    if (jvm != NULL) {
        if (jvm->AttachCurrentThread(&vm, NULL) == JNI_OK) {
            jniCls = vm->GetObjectClass(thizz);
            if (jniCls != NULL) {
                bdsCallBack = vm->GetStaticMethodID(jniCls, "_bdspiCallBack",
                                                    "(Ljava/lang/String;)V");
                if (bdsCallBack != NULL) {
                    jmsg = vm->NewStringUTF(msg);
                    vm->CallStaticVoidMethod(jniCls, bdsCallBack, jmsg);
                }
            }
            jvm->DetachCurrentThread();
        }
    }
}

int deinterleave_pcm_data(short *pIn, short **pOutMic, short **pOutRef, \
                        size_t sampleCnt, uint32_t micNum, uint32_t refNum) {
    if ((pIn == NULL) \
 || (pOutMic == NULL) \
 || (pOutRef == NULL)) {
        return -1;
    }

    for (size_t i = 0; i < sampleCnt; i++) {
        for (int inx = 0; inx < micNum; inx++) {
            short *micData = pOutMic[inx];
            *(micData + i) = *pIn++;
        }
        for (int inx = 0; inx < refNum; inx++) {
            short *refData = pOutRef[inx];
            *(refData + i) = *pIn++;
        }
    }

    return 0;
}

int mic_detect_test(AUDIO_HANDLE_T client, MIC_DETECT_PARAM_T *param, DETECT_RESULT *result) {
    void *handle = NULL;
    static int cnt = 0;
    short *mic_data = NULL;
    short *ref_data = NULL;

    handle = bd_mic_detect_init(param);
    if (handle == NULL) {
        return -1;
    }
    mic_data = (short *) malloc(sizeof(short) * MIC_8MS_SAMP_CNT * MIC_NUM);
    ref_data = (short *) malloc(sizeof(short) * MIC_8MS_SAMP_CNT * REF_NUM);
    memset(mic_data, 0, sizeof(short) * MIC_8MS_SAMP_CNT * MIC_NUM);
    memset(ref_data, 0, sizeof(short) * MIC_8MS_SAMP_CNT * REF_NUM);

    int nfrm = 0;
    bool last_flag = false;
    while (!forceExit) {
        if (0 == bds_audio_read(client, ORIG, szTmp, ONE_FRAME_SZ)) {
            /* deinterleave */
            short *mic_in[MIC_NUM] = {NULL};
            short *ref_in[REF_NUM] = {NULL};

            for (int inx = 0; inx < MIC_NUM; inx++) {
                mic_in[inx] = (short *) mic_data + inx * MIC_8MS_SAMP_CNT;
            }
            for (int inx = 0; inx < REF_NUM; inx++) {
                ref_in[inx] = (short *) ref_data + inx * MIC_8MS_SAMP_CNT;
            }
            deinterleave_pcm_data((short *) szTmp, mic_in, ref_in, \
                                MIC_8MS_SAMP_CNT, MIC_NUM, REF_NUM);

            if (nfrm == NFRM) {
                last_flag = true;
                bd_mic_detect_process(handle, mic_data, ref_data, last_flag, result);
                break;
            }
            bd_mic_detect_process(handle, mic_data, ref_data, last_flag, result);
            nfrm++;
            fwrite(szTmp, ONE_FRAME_SZ, 1, fd_rec);
        } else {
            forceExit = true;
            ALOGD("Read asr data failed\n");
            break;
        }
    }

    free(mic_data);
    free(ref_data);
    bd_mic_detect_uinit(handle);
    return 0;
}


/* 回调函数，回调返回后，event内存会被释放 */
int audio_client_callback(AudioClientEvent_t *event, void *arg) {
    if (event == NULL) {
        return -1;
    }
    WAKEUP_EVENT_T *wake_event = (WAKEUP_EVENT_T *) (event->event_data);
    DSP_EVENT_T *dsp_info = (DSP_EVENT_T *) (event->event_data);
    ALOGD("Received event code[0x%x]\n", event->event_code);
    switch (event->event_code) {
        case EVENT_RECORDER_START:
            ALOGD("Recorder started!\n");
            break;
        case EVENT_RECORDER_STOP:
            ALOGD("Recorder stopped!\n");
            break;
        case EVENT_RECORDER_ERROR:
            ALOGD("Recorder error!\n");
            break;
        case EVENT_RECORDER_FIRST_PKG:
            ALOGD("Recorder received first pkg!\n");
            break;
        case EVENT_WAKEUP:
            ALOGD("Wakeup triggered!\n");
            ALOGD("  wakeup words id = %d\n", wake_event->id);
            ALOGD("  wakeup words = %s\n", wake_event->wakeup_words);
            ALOGD("  wakeup location = %d\n", wake_event->location);
            ALOGD("  wakeup decoder param = %s\n", wake_event->decoder_param);
            ALOGD("  wakeup decoder param len= %d\n", wake_event->decoder_param_len);
            ALOGD("  wakeup begin seqence = %d\n", wake_event->wak_beg);
            ALOGD("  wakeup end seqence = %d\n", wake_event->wak_end);
            ALOGD("  wakeup backtrace seqence = %d\n", wake_event->audio_beg);
            //todo 回调给java端
            bds_callback(const_cast<char *>("1]wakeup"));
            break;
        case EVENT_VAD_BEGIN:
            ALOGD("VAD begin!\n");
            break;
        case EVENT_VAD_END:
            ALOGD("VAD end!\n");
            break;
        case EVENT_DSP_INFO:
            ALOGD("DSP info, err[0x%x], info[\"%s\"]!\n", dsp_info->err_code, dsp_info->info);
            break;
        case EVENT_VOLUME_INFO:
            ALOGD("Volume info!\n");
            break;
        default:
            break;
    }
    return 0;
}


extern "C" JNIEXPORT jint JNICALL
Java_com_mario_factorymictest_jni_FactoryMicTestJNI_wakeUpTest(
        JNIEnv *env,
        jobject /* this */) {

    int ret = 0;
    signal(SIGTERM, (__sighandler_t) handle_sig);
    signal(SIGINT, (__sighandler_t) handle_sig);
    signal(SIGABRT, (__sighandler_t) handle_sig);
    signal(SIGPIPE, SIG_IGN);

    pid_t tid = syscall(SYS_gettid);
    (void) setpriority(PRIO_PROCESS, tid, -10);
    AUDIO_CFG_PARAM_T cfg_param = {0};
    cfg_param.work_mode = FACTORY_MODE;
    cfg_param.param = NULL;
    cfg_param.param_len = 0;
    client = bds_audio_create(&cfg_param);
    if (client == NULL) {
        ALOGE("audio client create failed\n");
    }
    ALOGD("Bds audio create succeed[%p], version[%s]\n", \
            client, bds_audio_versoin(client));
    bds_audio_set_event_listener(client, &audio_client_callback);
    ret = bds_audio_start(client);
    ALOGE("ret = %d\n", ret)
    if (ret != 0) {
        ALOGD("bds_audio_start failed\n");
        ret = -1;
        goto EXIT;
    }
    ALOGD("bds_audio_start success\n");

    fd_rec = fopen(TEST_PCM_FILE, "wb");
    if (fd_rec == NULL) {
        ALOGD("fopen failed\n");
        ret = -1;
        goto EXIT;
    }
    (void) audio_wakeup_start(client);
    return ret;
    EXIT:
    if (client) {
        (void) audio_wakeup_stop(client);
        (void) bds_audio_stop(client);
        (void) bds_audio_destroy(client);
    }
    if (fd_rec) {
        fclose(fd_rec);
    }

//    param.sample_rate = 16000;
//    param.mic_num = MIC_NUM;
//    param.ref_num = REF_NUM;
//    param.thld[0] = db_to_ratio(-3.5); //3db
//    param.thld[1] = 0;
//    param.thld[2] = 0;
//    param.detect_flag = INCONSISTENT_DET;
//    mic_result = (DETECT_RESULT *)malloc(sizeof(DETECT_RESULT)*param.mic_num);
//    memset(mic_result, 0, sizeof(DETECT_RESULT) * param.mic_num);
//    if(mic_detect_test(client, &param, mic_result)) {
//        ALOGD("inconsistent test failed\n");
//    }
//    for(int inx = 0; inx < MIC_NUM; inx++) {
//        ALOGD("mic[%d] status: %d, energy_mean:%f\n", inx,
//              mic_result[inx].detect_status,\
//            mic_result[inx].energy_mean);
//    }
//    ALOGD("inconsistent test done\n");

    return ret;


}




extern "C" JNIEXPORT void JNICALL
Java_com_mario_factorymictest_jni_FactoryMicTestJNI_initJNI(JNIEnv *env, jobject thiz) {

    env->GetJavaVM(&jvm);
    thizz = env->NewGlobalRef(thiz);

//    DETECT_RESULT *mic_result = NULL;
//    MIC_DETECT_PARAM_T param = {0};

//    signal(SIGTERM, (__sighandler_t) handle_sig);
//    signal(SIGINT, (__sighandler_t) handle_sig);
//    signal(SIGABRT, (__sighandler_t) handle_sig);
//    signal(SIGPIPE, SIG_IGN);
//
//    pid_t tid = syscall(SYS_gettid);
//    (void) setpriority(PRIO_PROCESS, tid, -10);
//    AUDIO_CFG_PARAM_T cfg_param = {0};
//    cfg_param.work_mode = FACTORY_MODE;
//    cfg_param.param = NULL;
//    cfg_param.param_len = 0;
//    client = bds_audio_create(&cfg_param);
//    if (client == NULL) {
//        ALOGE("audio client create failed\n");
//    }
//    ALOGD("Bds audio create succeed[%p], version[%s]\n", \
//            client, bds_audio_versoin(client));
//    bds_audio_set_event_listener(client, &audio_client_callback);
//    int ret = bds_audio_start(client);
//    ALOGE("ret = %d\n", ret)
//    if (ret != 0) {
//        ALOGD("bds_audio_start failed\n");
//        ret = -1;
//        goto EXIT;
//    }
//    ALOGD("bds_audio_start success\n");
//
//    //(void) audio_wakeup_start(client);
//    fd_rec = fopen(TEST_PCM_FILE, "wb");
//    if (fd_rec == NULL) {
//        ALOGD("fopen failed\n");
//        ret = -1;
//        goto EXIT;
//    }
//    return;
//    EXIT:
//    if (client) {
//        (void) audio_wakeup_stop(client);
//        (void) bds_audio_stop(client);
//        (void) bds_audio_destroy(client);
//    }
//    if (fd_rec) {
//        fclose(fd_rec);
//    }
}



extern "C" JNIEXPORT void JNICALL
Java_com_mario_factorymictest_jni_FactoryMicTestJNI_wakeUpStop(JNIEnv *env, jobject thiz) {
    wakeup_stop();
}


extern "C" JNIEXPORT void JNICALL
Java_com_mario_factorymictest_jni_FactoryMicTestJNI_release(JNIEnv *env, jobject thiz) {
    clear();
}




extern "C" JNIEXPORT void JNICALL
Java_com_mario_factorymictest_jni_FactoryMicTestJNI_inconsistentTest(JNIEnv *env, jobject thiz) {


    MIC_DETECT_PARAM_T param = {0};
    DETECT_RESULT *detect_result = NULL;
    memset(str,0,300);
    int total_chns = 0;
    signal(SIGTERM, (__sighandler_t) handle_sig);
    signal(SIGINT, (__sighandler_t) handle_sig);
    signal(SIGABRT, (__sighandler_t) handle_sig);
    signal(SIGPIPE, SIG_IGN);

    pid_t tid = syscall(SYS_gettid);
    (void) setpriority(PRIO_PROCESS, tid, -10);
    AUDIO_CFG_PARAM_T cfg_param = {0};
    cfg_param.work_mode = FACTORY_MODE;
    cfg_param.param = NULL;
    cfg_param.param_len = 0;
    client = bds_audio_create(&cfg_param);
    if (client == NULL) {
        ALOGE("audio client create failed\n");
    }
    ALOGD("Bds audio create succeed[%p], version[%s]\n", \
            client, bds_audio_versoin(client));
    bds_audio_set_event_listener(client, &audio_client_callback);
    int ret = bds_audio_start(client);
    ALOGE("ret = %d\n", ret)
    if (ret != 0) {
        ALOGD("bds_audio_start failed\n");
        ret = -1;

    }
    ALOGD("bds_audio_start success\n");

    fd_rec = fopen(TEST_PCM_FILE, "wb");
    if (fd_rec == NULL) {
        ALOGD("fopen failed\n");
        ret = -1;

    }

    param.sample_rate = 16000;
    param.mic_num = MIC_NUM;
    param.ref_num = REF_NUM;
    param.thld[0] = db_to_ratio(-3.5); //3db
    param.thld[1] = db_to_ratio(-1.5); //1.5db
    param.thld[2] = 0;
    param.detect_flag = INCONSISTENT_DET;
    total_chns = param.mic_num + param.ref_num;
    detect_result = (DETECT_RESULT *)malloc(sizeof(DETECT_RESULT) * total_chns);
    memset(detect_result, 0, sizeof(DETECT_RESULT) * total_chns);
    if(mic_detect_test(client, &param, detect_result)) {
        ALOGD("inconsistent test failed\n");
    }
    str[0] = '2';
    str[1] = ']';
    for(int inx = 0; inx < MIC_NUM; inx++) {
        ALOGD("mic[%d] status: %d, energy_mean:%f\n", inx,
              detect_result[inx].detect_status,\
            detect_result[inx].energy_mean);
        sprintf(str + strlen(str), "%d&&%f,", detect_result[inx].detect_status,
                detect_result[inx].energy_mean);
    }
    for(int inx = MIC_NUM; inx < total_chns; inx++) {
        ALOGD("ref[%d] status: %d, energy_mean:%f, energy_peak:%f\n", inx,
              detect_result[inx].detect_status,\
            detect_result[inx].energy_mean,\
            detect_result[inx].energy_peak);
        sprintf(str + strlen(str), "%d&&%f,", detect_result[inx].detect_status,
                detect_result[inx].energy_mean);
    }
    ALOGD("inconsistent test done\n");




//    param.sample_rate = 16000;
//    param.mic_num = MIC_NUM;
//    param.ref_num = REF_NUM;
//    param.thld[0] = db_to_ratio(-3.5); //3db
//    param.thld[1] = db_to_ratio(-1.5);
//    param.thld[2] = 0;
//    param.detect_flag = INCONSISTENT_DET;
//    total_chns = param.mic_num + param.ref_num;
//    mic_result = (DETECT_RESULT *) malloc(sizeof(DETECT_RESULT) * total_chns);
//    memset(mic_result, 0, sizeof(DETECT_RESULT) * total_chns);
//    if (mic_detect_test(client, &param, mic_result)) {
//        ALOGD("inconsistent test failed\n");
//    }
//    memset(str, 0, sizeof(str));
//    str[0] = '2';
//    str[1] = ']';
//    for (int inx = 0; inx < MIC_NUM; inx++) {
//        ALOGD("mic[%d] status: %d, energy_mean:%f\n", inx,
//              mic_result[inx].detect_status, \
//            mic_result[inx].energy_mean);
//        sprintf(str + strlen(str), "%d&&%f,", mic_result[inx].detect_status,
//                mic_result[inx].energy_mean);
//
//    }
//
//
//    for(int inx = MIC_NUM; inx < total_chns; inx++) {
//        ALOGD("ref[%d] status: %d, energy_mean:%f, energy_peak:%f\n", inx,
//              mic_result[inx].detect_status,
//              mic_result[inx].energy_mean,
//              mic_result[inx].energy_peak);
//
//    }

//    char newString[302];
//    newString[0] = '2';
//    newString[1] = ']';
//    memcpy(newString+2,str,strlen(str));


    ALOGE("%s", str)

    ALOGD("inconsistent test done\n");

    if (detect_result) {
        free(detect_result);
    }

    //bds_callback(str);
    jclass jniClazz = env->GetObjectClass(thiz);
    if (jniClazz != NULL) {
        jmethodID mainBdsCallBack = env->GetStaticMethodID(jniClazz, "_bdspiCallBack",
                                                           "(Ljava/lang/String;)V");
        if (mainBdsCallBack != NULL) {
            jstring jmsgg = env->NewStringUTF(str);
            env->CallStaticVoidMethod(jniClazz, mainBdsCallBack, jmsgg);

            env->DeleteLocalRef(jmsgg);
        }
    }


}




