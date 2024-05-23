#include <stdlib.h>                                                         
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include "../include/utils/logger.h"

int i = 0;
//互斥锁 跟java类似，如果多个锁，也可以死锁。
pthread_mutex_t pthreadMutex;

void* test2_thr_fun(void* arg){
	//加锁
	pthread_mutex_lock(&pthreadMutex);
	char* no = (char*)arg;
	for(;i < 5; i++){
		LOGI("%s thread, i:%d\n",no,i);
		usleep(1000);
	}
	i=0;
	//解锁
	pthread_mutex_unlock(&pthreadMutex);
}

JNIEXPORT void JNICALL
Java_com_dongnaoedu_dnffmpegplayer_PThreadTest_test2(JNIEnv *env, jobject thiz) {
    pthread_t tid1, tid2;
    //初始化互斥锁
    pthread_mutex_init(&pthreadMutex,NULL);

    pthread_create(&tid1,NULL,test2_thr_fun,"No1");
    pthread_create(&tid2,NULL,test2_thr_fun,"No2");

    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);

    //销毁互斥锁
    pthread_mutex_destroy(&pthreadMutex);
}