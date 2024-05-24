extern "C" {
//#include "wavlib.h"
#include "../../../../../transcode-1.1.7/avilib/wavlib.h"
}
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))


WAV wav; //wav文件指针
SLObjectItf engineObject; //引擎对象
SLEngineItf engineInterface; //引擎接口
SLObjectItf outputMixObject; //混音器
SLObjectItf audioPlayerObject; //播放器对象
SLAndroidSimpleBufferQueueItf andioPlayerBufferQueueItf;	//缓冲器队列接口
SLPlayItf audioPlayInterface;	//播放接口


unsigned char *buffer; //缓冲区
size_t bufferSize;	   //缓冲区大小

//上下文
struct PlayerContext{
	WAV wav;
	unsigned char *buffer;
	size_t bufferSize;

	PlayerContext(WAV wav,
			unsigned char *buffer,
			size_t bufferSize){
		this->wav = wav;
		this->buffer = buffer;
		this->bufferSize = bufferSize;
	}
};

//创建音频播放对象
void CreateBufferQueueAudioPlayer(
        WAV wav,
        SLEngineItf engineEngine,
        SLObjectItf outputMixObject,
        SLObjectItf &audioPlayerObject){

    // Android针对数据源的简单缓冲区队列定位器
    SLDataLocator_AndroidSimpleBufferQueue dataSourceLocator = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, // 定位器类型
            1                                        // 缓冲区数
    };

    // PCM数据源格式
    SLDataFormat_PCM dataSourceFormat = {
            SL_DATAFORMAT_PCM,        // 格式类型
            wav_get_channels(wav),    // 通道数
            wav_get_rate(wav) * (unsigned int)1000, // 毫赫兹/秒的样本数
            wav_get_bits(wav),        // 每个样本的位数
            wav_get_bits(wav),        // 容器大小
            SL_SPEAKER_FRONT_CENTER,  // 通道屏蔽
            SL_BYTEORDER_LITTLEENDIAN // 字节顺序
    };

    // 数据源是含有PCM格式的简单缓冲区队列
    SLDataSource dataSource = {
            &dataSourceLocator, // 数据定位器
            &dataSourceFormat   // 数据格式
    };

    // 针对数据接收器的输出混合定位器
    SLDataLocator_OutputMix dataSinkLocator = {
            SL_DATALOCATOR_OUTPUTMIX, // 定位器类型
            outputMixObject           // 输出混合
    };

    // 数据定位器是一个输出混合
    SLDataSink dataSink = {
            &dataSinkLocator, // 定位器
            0                 // 格式
    };

    // 需要的接口
    SLInterfaceID interfaceIds[] = {
            SL_IID_BUFFERQUEUE
    };

    // 需要的接口，如果所需要的接口不要用，请求将失败
    SLboolean requiredInterfaces[] = {
            SL_BOOLEAN_TRUE // for SL_IID_BUFFERQUEUE
    };

    // 创建音频播放器对象
    SLresult result = (*engineEngine)->CreateAudioPlayer(
            engineEngine,
            &audioPlayerObject,
            &dataSource,
            &dataSink,
            ARRAY_LEN(interfaceIds),
            interfaceIds,
            requiredInterfaces);
}

//打开文件
WAV OpenWaveFile(JNIEnv *env,jstring jFileName){
	const char *cFileName = env->GetStringUTFChars(jFileName,JNI_FALSE);
	WAVError err;
	WAV wav = wav_open(cFileName,WAV_READ,&err);

	LOGI("%d",wav_get_bitrate(wav));
	env->ReleaseStringUTFChars(jFileName,cFileName);
	if(wav == 0){
		LOGE("%s",wav_strerror(err));
	}
	return wav;
}

//关闭文件
void CloseWaveFile(WAV wav){
	wav_close(wav);
}

//实现对象
void RealizeObject(SLObjectItf object){
	//非异步（阻塞）
	(*object)->Realize(object,SL_BOOLEAN_FALSE);
}

//回调函数
void PlayerCallBack(SLAndroidSimpleBufferQueueItf andioPlayerBufferQueue,void *context){
	PlayerContext* ctx = (PlayerContext*)context;
	//读取数据
	ssize_t readSize = wav_read_data(ctx->wav,ctx->buffer,ctx->bufferSize);
	if(0 < readSize){
		(*andioPlayerBufferQueue)->Enqueue(andioPlayerBufferQueue,ctx->buffer,readSize);
	}else{
		//destroy context
		CloseWaveFile(ctx->wav); //关闭文件
		delete ctx->buffer; //释放缓存
	}
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dongnaoedu_opensl_AudioPlayer_play(JNIEnv *env, jclass clazz, jstring jFileName){
	//1.打开文件
	WAV wav = OpenWaveFile(env,jFileName);

	//2.创建OpenSL ES引擎
	//OpenSL ES在Android平台下默认是线程安全的，这样设置是为了兼容其他平台
	SLEngineOption options[] = {
		{(SLuint32)SL_ENGINEOPTION_THREADSAFE, (SLuint32)SL_BOOLEAN_TRUE}
	};
	slCreateEngine(&engineObject,ARRAY_LEN(engineObject),options,0,0,0); //没有接口

	//实例化对象
	//对象创建之后，处于未实例化状态，对象虽然存在但未分配任何资源，使用前先实例化（使用完之后destroy）
	RealizeObject(engineObject);

	//3.获取引擎接口
	(*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineInterface);

	//4.创建输出混音器
	(*engineInterface)->CreateOutputMix(engineInterface,&outputMixObject,0,0,0); //没有接口

	//实例化混音器
	RealizeObject(outputMixObject);

	//5.创建缓冲区保存读取到的音频数据库
	//缓冲区的大小
	bufferSize = wav_get_channels(wav) * wav_get_rate(wav) * wav_get_bits(wav);
	buffer = new unsigned char[bufferSize];

	//6.创建带有缓冲区队列的音频播放器
	CreateBufferQueueAudioPlayer(wav,engineInterface,outputMixObject,audioPlayerObject);

	//实例化音频播放器
	RealizeObject(audioPlayerObject);

	//7.获得缓冲区队列接口Buffer Queue Interface
	//通过缓冲区队列接口对缓冲区进行排序播放
	(*audioPlayerObject)->GetInterface(audioPlayerObject,SL_IID_BUFFERQUEUE,&andioPlayerBufferQueueItf);

	//8.注册音频播放器回调函数
	//当播放器完成对前一个缓冲区队列的播放时，回调函数会被调用，然后我们又继续读取音频数据，直到结束
	//上下文，包裹参数方便再回调函数中使用
	PlayerContext *ctx = new PlayerContext(wav,buffer,bufferSize);
	(*andioPlayerBufferQueueItf)->RegisterCallback(andioPlayerBufferQueueItf,PlayerCallBack,ctx);

	//9.获取Play Interface通过对SetPlayState函数来启动播放音乐
	//一旦播放器被设置为播放状态，该音频播放器开始等待缓冲区排队就绪
	(*audioPlayerObject)->GetInterface(audioPlayerObject,SL_IID_PLAY,&audioPlayInterface);
	//设置播放状态
	(*audioPlayInterface)->SetPlayState(audioPlayInterface,SL_PLAYSTATE_PLAYING);

	//10.开始，让第一个缓冲区入队
	PlayerCallBack(andioPlayerBufferQueueItf,ctx);


	//关闭文件
	//CloseWaveFile(wav);
}