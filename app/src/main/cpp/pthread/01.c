#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include "../include/utils/logger.h"

void* test1_thr_fun(void* arg){
	char* no = (char*)arg;
	int i = 0;
	for(; i < 10; i++){
        LOGI("%s mythread, i:%d\n",no,i);
		if(i==5){
			//线程退出（自杀），指定的参数是return回去的值。
			pthread_exit((void *) 2);
			//任何一个线程可以结束另一个线程  他杀pthread_cancel(tid)			
		}
	}	
	return (void *) 1;
}


JNIEXPORT void JNICALL
Java_com_dongnaoedu_dnffmpegplayer_PThreadTest_test1(JNIEnv *env, jobject thiz) {
    LOGI("main thread\n");
    //线程id
    pthread_t tid;
    //线程的属性，NULL默认属性
    //test1_thr_fun，线程创建之后执行的函数，"1"是给thr_fun传的参数
    pthread_create(&tid, NULL, test1_thr_fun, "1");
    void* rval;
	//pthread_join 本线程一直等待tid线程结束
	//test1_thr_fun的值与pthread_exit退出时传入的参数，都作为pthread_join函数第二个参数的内容,不需要返回值可以写NULL
    pthread_join(tid,&rval);
    LOGI("myrval:%d\n",(int)rval);
}
