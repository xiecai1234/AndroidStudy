#include <stdio.h>
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>
#include <jni.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

JavaVM *javaVM;
jobject uuidutils_class_global;
jmethodID uuidutils_get_mid;

//动态库加载时会自动执行JNI_OnLoad
//Android SDK 2.2之后才有，2.2没有这个函数
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
	LOGI("%s","JNI_OnLoad");
    // 方式1赋值
//	javaVM = vm;
    //告诉VM此C组件使用哪一个JNI版本。
	return JNI_VERSION_1_4;
}

void* th_fun(void* arg){
    const char* sss = (const char *)arg;
	int i;
	for (i = 0; i < 5; i++) {
		JNIEnv* env;
        //通过JavaVM关联当前线程，获取当前线程的JNIEnv
		//关联参数，可以设置为NULL
		JavaVMAttachArgs args = {JNI_VERSION_1_4, "my_thread", NULL};
		(*javaVM)->AttachCurrentThread(javaVM,&env,&args);
//		(*javaVM)->AttachCurrentThread(javaVM,&env,NULL);
		jobject uuid_jstr = (*env)->CallStaticObjectMethod(env,uuidutils_class_global,uuidutils_get_mid);
		const char* uuid_cstr = (*env)->GetStringUTFChars(env,uuid_jstr,NULL);
		LOGI("uuid:%s",uuid_cstr);
		//退出线程
		if(i == 2){
//            pthread_exit((void*)0);
			goto end;
		}
		sleep(1);
	}
//正常或异常退出都执行end语句
end:
    LOGI("%s","reach end");
    //解除关联 必须 否则会报错
	(*javaVM)->DetachCurrentThread(javaVM);
	pthread_exit((void*)0);
}

//JavaVM 代表的是Java虚拟机，所有的工作（如访问java对象等）都是从JavaVM开始，只有一个，所有线程都是同一个JavaVM
//可以通过JavaVM获取到每个线程关联的JNIEnv
//如何获取JavaVM？
//方式1.在JNI_OnLoad函数中获取
//方式2.(*env)->GetJavaVM(env,&javaVM);

//每个线程都有独立的JNIEnv


//初始化
JNIEXPORT void JNICALL Java_com_dongnaoedu_androidlinux_PosixThread_init
  (JNIEnv *env, jobject jobj){
	//获取class必须要在主线程中,否则获取不到
	jclass uuidutils_class_tmp = (*env)->FindClass(env,"com/dongnaoedu/androidlinux/UUIDUtils");
	//创建全局引用  局部引用是没法跨线程使用的
	uuidutils_class_global = (*env)->NewGlobalRef(env,uuidutils_class_tmp);
	//获取jmethodId也可以在子线程中
	uuidutils_get_mid = (*env)->GetStaticMethodID(env,uuidutils_class_global,"get","()Ljava/lang/String;");
}

//销毁
JNIEXPORT void JNICALL Java_com_dongnaoedu_androidlinux_PosixThread_destroy
  (JNIEnv *env, jobject jobj){
	//释放全局引用
	(*env)->DeleteGlobalRef(env,uuidutils_class_global);
}

JNIEXPORT void JNICALL Java_com_dongnaoedu_androidlinux_PosixThread_pthread
  (JNIEnv *env, jobject jobj){
    // 方式2
	(*env)->GetJavaVM(env,&javaVM);
    //创建子线程 不建议在这里将env传递给子线程
	pthread_t tid;
	pthread_create(&tid, NULL,th_fun,(void*)"NO1");
}


