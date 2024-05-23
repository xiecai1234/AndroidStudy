#include <stdlib.h>                                                      
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include "../include/utils/logger.h"

//消费者数量
#define CONSUMER_NUM 50
//生产者数量
#define PRODUCER_NUM 10
//修改数量观察

pthread_t pids[CONSUMER_NUM+PRODUCER_NUM];

//产品队列 此处用整数简单模拟
int ready = 0;

//消费者线程发现没产品时，进入等待，生成者生产出产品时通知消费者去取。
//互斥锁
pthread_mutex_t mutex;
//条件变量
pthread_cond_t has_product;

//生产
void* producer(void* arg){
	int no = (int)arg;
	//条件变量
	for(;;){
		pthread_mutex_lock(&mutex);
		//往队列中添加产品
		ready++;
		LOGI("生产者%d, 生产了一个产品，当前产品数量:%d\n",no,ready);
		//fflush(NULL);
		//通知消费者，有新的产品可以消费了
		//会阻塞输出 输出在缓存了，屏幕上没输出
		pthread_cond_signal(&has_product);
		LOGI("生产者%d, 通知消费者去消费\n",no);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
}

//消费者
void* consumer(void* arg){
	int num = (int)arg;
	for(;;){
		pthread_mutex_lock(&mutex);
		// 不能用if
        // 除了pthread_cond_signal，superious wake ‘惊群效应’ 也有可能会解除阻塞。所以用while，记住就行。
		while(ready==0){
			//没有产品，继续等待
			//1.一直阻塞，等待has_product被唤醒
			//2.释放互斥锁，相当于调用pthread_mutex_unlock，生产者和其他消费者才可以再获取到锁。
			//3.被唤醒时，解除阻塞，重新申请获得互斥锁，相当于调用pthread_mutex_lock
			LOGI("消费者%d等待中....\n",num);
			pthread_cond_wait(&has_product,&mutex);
		}
		//有产品，消费产品
		ready--;
		LOGI("消费者%d消费了一个产品,当前剩余产品数量:%d\n",num, ready);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
}


JNIEXPORT void JNICALL
Java_com_dongnaoedu_dnffmpegplayer_PThreadTest_test3(JNIEnv *env, jobject thiz) {
    //初始化互斥锁和条件变量
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&has_product,NULL);
    LOGI("init\n");

    int i;
    for(i=0; i<PRODUCER_NUM;i++){
        //生产者线程
        LOGI("创建生产者线程%d\n",i);
        pthread_create(&pids[i],NULL,producer,(void*)i);
    }

    for(i=0; i<CONSUMER_NUM;i++){
        //消费者线程
        LOGI("创建消费者线程%d\n",i);
        pthread_create(&pids[PRODUCER_NUM+i],NULL,consumer,(void*)i);
    }

    //等待
    sleep(10);
    for(i=0; i<PRODUCER_NUM+CONSUMER_NUM;i++){
        pthread_join(pids[i],NULL);
    }

    //销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&has_product);
}