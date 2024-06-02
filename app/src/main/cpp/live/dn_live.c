#include <stdlib.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <pthread.h>
#include <string.h>
#include "x264.h"
#include "rtmp.h"
#include "faac.h"
#include "../include/queue.h"
#include "../include/utils/logger.h"
#include "../include/utils/common.h"


unsigned int start_time;
//线程处理
pthread_mutex_t mutex;
pthread_cond_t cond;
//rtmp流媒体地址
char *rtmp_path;
//是否直播
int is_pushing = FALSE;

jobject jobj_push_native; //Global ref
jclass jcls_push_native; //Global ref
jmethodID jmid_throw_native_error; //local ref
JavaVM *javaVM;

extern void setVideoOptions(JNIEnv *env, jobject jobj, jint width, jint height, jint bitrate, jint fps);
extern void setAudioOptions(JNIEnv *env, jobject jobj, jint sampleRateInHz, jint numChannels);
extern void add_aac_sequence_header();
extern void add_264_sequence_header(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);
extern void fireVideo(JNIEnv *env, jobject jobj, jbyteArray buffer);
extern void fireAudio(JNIEnv *env, jobject jobj, jbyteArray buffer, jint len);
//获取JavaVM
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_4;
}

/**
 * 向Java层发送错误信息
 */
void throwNativeError(JNIEnv *env, int code) {
    (*env)->CallVoidMethod(env, jobj_push_native, jmid_throw_native_error, code);
}

/**
 * 从队列中不断拉取RTMPPacket发送给流媒体服务器）
 */
void *push_thread() {
    JNIEnv *env;//获取当前线程JNIEnv
    (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);

    //建立RTMP连接
    RTMP *rtmp = RTMP_Alloc();
    if (!rtmp) {
        LOGE("rtmp初始化失败")
        goto end;
    }
    RTMP_Init(rtmp);
    rtmp->Link.timeout = 5; //连接超时的时间
    //设置流媒体地址
    RTMP_SetupURL(rtmp, rtmp_path);
    //发布rtmp数据流
    RTMP_EnableWrite(rtmp);
    //建立连接
    if (!RTMP_Connect(rtmp, NULL)) {
        LOGE("%s", "RTMP 连接失败")
        throwNativeError(env, CONNECT_FAILED);
        goto end;
    }
    //计时
    start_time = RTMP_GetTime();
    if (!RTMP_ConnectStream(rtmp, 0)) { //连接流
        LOGE("%s", "RTMP ConnectStream failed")
        throwNativeError(env, CONNECT_FAILED);
        goto end;
    }
    is_pushing = TRUE;
    //发送AAC头信息
    add_aac_sequence_header();

    while (is_pushing) {
        //发送
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        //取出队列中的RTMPPacket
        RTMPPacket *packet = queue_get_first();
        if (packet) {
            queue_delete_first(); //移除
            packet->m_nInfoField2 = rtmp->m_stream_id; //RTMP协议，stream_id数据
            int i = RTMP_SendPacket(rtmp, packet, TRUE); //TRUE放入librtmp队列中，并不是立即发送
            if (!i) {
                LOGE("RTMP 断开")
                RTMPPacket_Free(packet);
                pthread_mutex_unlock(&mutex);
                goto end;
            } else {
                LOGI("%s", "rtmp send packet")
            }
            RTMPPacket_Free(packet);
        }

        pthread_mutex_unlock(&mutex);
    }
    end:
    LOGI("%s", "释放资源")
    free(rtmp_path);
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    (*javaVM)->DetachCurrentThread(javaVM);
    return 0;
}

JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_startPush
        (JNIEnv *env, jobject jobj, jstring url_jstr) {
    //jobj(PushNative对象)
    jobj_push_native = (*env)->NewGlobalRef(env, jobj);

    jclass jcls_push_native_tmp = (*env)->GetObjectClass(env, jobj);
    jcls_push_native = (*env)->NewGlobalRef(env, jcls_push_native_tmp);
    //PushNative.throwNativeError
    jmid_throw_native_error = (*env)->GetMethodID(env, jcls_push_native_tmp, "throwNativeError",
                                                  "(I)V");
    //初始化的操作  TODO
    const char *url_cstr = (*env)->GetStringUTFChars(env, url_jstr, NULL);
    //复制url_cstr内容到rtmp_path
    rtmp_path = malloc(strlen(url_cstr) + 1);
    memset(rtmp_path, 0, strlen(url_cstr) + 1);
    memcpy(rtmp_path, url_cstr, strlen(url_cstr));

    //初始化互斥锁与条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    //创建队列
    create_queue();
    //启动消费者线程（从队列中不断拉取RTMPPacket发送给流媒体服务器）
    pthread_t push_thread_id;
    pthread_create(&push_thread_id, NULL, push_thread, NULL);

    (*env)->ReleaseStringUTFChars(env, url_jstr, url_cstr);
}

JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_stopPush
        (JNIEnv *env, jobject jobj) {
    is_pushing = FALSE;
}

JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_release
        (JNIEnv *env, jobject jobj) {
    (*env)->DeleteGlobalRef(env, jcls_push_native);
    (*env)->DeleteGlobalRef(env, jobj_push_native);
    //TODO
    (*env)->DeleteGlobalRef(env, jmid_throw_native_error);
}

/**
 * 设置视频参数
 */
JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_setVideoOptions
        (JNIEnv *env, jobject jobj, jint width, jint height, jint bitrate, jint fps) {
    setVideoOptions(env, jobj, width, height, bitrate, fps);
}

/**
 * 音频编码器配置
 */
JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_setAudioOptions
        (JNIEnv *env, jobject jobj, jint sampleRateInHz, jint numChannels) {
    setAudioOptions(env, jobj, sampleRateInHz, numChannels);
}

/**
 * 将采集到视频数据进行编码
 */
JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_fireVideo
        (JNIEnv *env, jobject jobj, jbyteArray buffer) {
    fireVideo(env, jobj, buffer);
}

/**
 * 对音频采样数据进行AAC编码
 */
JNIEXPORT void JNICALL Java_com_dongnaoedu_live_jni_PushNative_fireAudio
        (JNIEnv *env, jobject jobj, jbyteArray buffer, jint len) {
    fireAudio(env, jobj, buffer, len);
}
