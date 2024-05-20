#include <android/log.h>
#include <jni.h>

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRME_SIZE 48000 * 4

//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"

JNIEXPORT void JNICALL Java_com_dongnaoedu_dnffmpegplayer_JasonPlayer_sound
  (JNIEnv *env, jobject jobj, jstring input_jstr, jstring output_jstr){
	const char* input_cstr = (*env)->GetStringUTFChars(env,input_jstr,NULL);
	const char* output_cstr = (*env)->GetStringUTFChars(env,output_jstr,NULL);
	LOGI("%s","sound");
	//注册组件
	av_register_all();
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	//打开音频文件
	if(avformat_open_input(&pFormatCtx,input_cstr,NULL,NULL) != 0){
		LOGI("%s","无法打开音频文件");
		return;
	}
	//获取输入文件信息
	if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
		LOGI("%s","无法获取输入文件信息");
		return;
	}
	//获取音频流索引位置
	int i = 0, audio_stream_idx = -1;
	for(; i < pFormatCtx->nb_streams;i++){
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_stream_idx = i;
			break;
		}
	}

	//获取解码器
	AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
	AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
	if(codec == NULL){
		LOGI("%s","无法获取解码器");
		return;
	}
	//打开解码器
	if(avcodec_open2(codecCtx,codec,NULL) < 0){
		LOGI("%s","无法打开解码器");
		return;
	}
	//压缩数据
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//解压缩数据
	AVFrame *frame = av_frame_alloc();
	//frame->16bit 44100 PCM 统一音频采样格式与采样率
	SwrContext *swrCtx = swr_alloc();

	//重采样设置参数-------------start
	//输入的采样格式
	enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
	//输出采样格式16bit PCM
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	//输入采样率
	int in_sample_rate = codecCtx->sample_rate;
	//输出采样率
	int out_sample_rate = 44100;
	//获取输入的声道布局
	//根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
	//av_get_default_channel_layout(codecCtx->channels);
	uint64_t in_ch_layout = codecCtx->channel_layout;
	//输出的声道布局（立体声）
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

	swr_alloc_set_opts(swrCtx,
		  out_ch_layout,out_sample_fmt,out_sample_rate,
		  in_ch_layout,in_sample_fmt,in_sample_rate,
		  0, NULL);
	swr_init(swrCtx);

	//输出的声道个数
	int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

	//重采样设置参数-------------end

	//16bit 44100 PCM 数据
	uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);

	FILE *fp_pcm = fopen(output_cstr,"wb");

	int got_frame = 0,index = 0, ret;
	//不断读取压缩数据
	while(av_read_frame(pFormatCtx,packet) >= 0){
		//解码
		ret = avcodec_decode_audio4(codecCtx,frame,&got_frame,packet);

		if(ret < 0){
			LOGI("%s","解码完成");
		}
		//解码一帧成功
		if(got_frame > 0){
			LOGI("解码：%d",index++);
			swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,frame->data,frame->nb_samples);
			//获取sample的size
			int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
					frame->nb_samples, out_sample_fmt, 1);
			fwrite(out_buffer,1,out_buffer_size,fp_pcm);
		}

		av_free_packet(packet);
	}

	fclose(fp_pcm);
	av_frame_free(&frame);
	av_free(out_buffer);

	swr_free(&swrCtx);
	avcodec_close(codecCtx);
	avformat_close_input(&pFormatCtx);

	(*env)->ReleaseStringUTFChars(env,input_jstr,input_cstr);
	(*env)->ReleaseStringUTFChars(env,output_jstr,output_cstr);

}
