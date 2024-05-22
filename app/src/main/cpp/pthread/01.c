#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include <android/log.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);
void* thr_fun(void* arg){
	char* no = (char*)arg;
	int i = 0;
	for(; i < 10; i++){
        LOGI("%s mythread, i:%d\n",no,i);
		if(i==5){
			//线程退出（自杀）
			pthread_exit((void *) 2);
			//他杀pthread_cancel			
		}
	}	
	return (void *) 1;
}


JNIEXPORT void JNICALL
Java_com_dongnaoedu_dnffmpegplayer_PThreadTest_test(JNIEnv *env, jobject thiz) {
    printf("main thread\n");
    //线程id
    pthread_t tid;
    //线程的属性，NULL默认属性
    //thr_fun，线程创建之后执行的函数
    pthread_create(&tid,NULL,thr_fun,"1");
    void* rval;
    //等待tid线程结束
    //thr_fun与退出时传入的参数，都作为第二个参数的内容
    pthread_join(tid,&rval);
    LOGI("myrval:%d\n",(int)rval);
}
