#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);


#include <stdio.h>
#include <string.h>

JNIEXPORT void JNICALL
Java_com_dongnaoedu_dnffmpegplayer_JasonPlayer_locationError(JNIEnv *env, jobject thiz) {
    char *addr = NULL;
    // 会报错
    strcpy(addr, "crash here");
}


