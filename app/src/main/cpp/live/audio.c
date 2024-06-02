//
// Created by xiecaibao on 2024/5/31.
//
#include <malloc.h>
#include <string.h>
#include <jni.h>
#include "faac.h"
#include "rtmp.h"
#include "../include/utils/logger.h"
#include "../include/utils/common.h"

extern unsigned int start_time;
extern void add_rtmp_packet();

//faac音频编码处理器
faacEncHandle audio_encode_handle;
unsigned long nInputSamples; //输入的采样个数
unsigned long nMaxOutputBytes; //编码输出之后的字节数

void setAudioOptions(JNIEnv *env, jobject jobj, jint sampleRateInHz, jint numChannels) {
    LOGI("setAudioOptions");
    audio_encode_handle = faacEncOpen(sampleRateInHz, numChannels, &nInputSamples,
                                      &nMaxOutputBytes);
    if (!audio_encode_handle) {
        LOGE("音频编码器打开失败");
        return;
    }
    //设置音频编码参数
    faacEncConfigurationPtr p_config = faacEncGetCurrentConfiguration(audio_encode_handle);
    p_config->mpegVersion = MPEG4;
    p_config->allowMidside = 1;
    p_config->aacObjectType = LOW;
    p_config->outputFormat = 0; //输出是否包含ADTS头
    p_config->useTns = 1; //时域噪音控制,大概就是消爆音
    p_config->useLfe = 0;
//	p_config->inputFormat = FAAC_INPUT_16BIT;
    p_config->quantqual = 100;
    p_config->bandWidth = 0; //频宽
    p_config->shortctl = SHORTCTL_NORMAL;

    if (!faacEncSetConfiguration(audio_encode_handle, p_config)) {
        LOGE("%s", "音频编码器配置失败..");
        throwNativeError(env, INIT_FAILED);
        return;
    }

    LOGI("%s", "音频编码器配置成功");
}
/**
 * 添加AAC头信息
 */
void add_aac_sequence_header() {
    LOGI("add_aac_sequence_header");
    //获取aac头信息的长度
//    unsigned char *buf;
//    unsigned long len; //长度
//    faacEncGetDecoderSpecificInfo(audio_encode_handle, &buf, &len);
//    int body_size = 2 + len;
//    RTMPPacket *packet = malloc(sizeof(RTMPPacket));
//    //RTMPPacket初始化
//    RTMPPacket_Alloc(packet, body_size);
//    RTMPPacket_Reset(packet);
//    char *body = packet->m_body;
//    //头信息配置
//    /*AF 00 + AAC RAW data*/
//    body[0] = 0xAF;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
//    body[1] = 0x00;//AACPacketType:0表示AAC sequence header
//    memcpy(&body[2], buf, len); /*spec_buf是AAC sequence header数据*/
//    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
//    packet->m_nBodySize = body_size;
//    packet->m_nChannel = 0x04;
//    packet->m_hasAbsTimestamp = 0;
//    packet->m_nTimeStamp = 0;
//    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
//    add_rtmp_packet(packet);
//    free(buf);
}

/**
 * 添加AAC rtmp packet
 */
void add_aac_body(unsigned char *buf, int len) {
    LOGI("add_aac_body");
    int body_size = 2 + len;
    RTMPPacket *packet = malloc(sizeof(RTMPPacket));
    //RTMPPacket初始化
    RTMPPacket_Alloc(packet, body_size);
    RTMPPacket_Reset(packet);
    char *body = packet->m_body;
    //头信息配置
    /*AF 00 + AAC RAW data*/
    body[0] = 0xAF;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
    body[1] = 0x01;//AACPacketType:1表示AAC raw
    memcpy(&body[2], buf, len); /*spec_buf是AAC raw数据*/
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = RTMP_GetTime() - start_time;
    add_rtmp_packet(packet);
}

void fireAudio(JNIEnv *env, jobject jobj, jbyteArray buffer, jint len){
    LOGI("fireAudio");
//    int *pcmbuf;
//    unsigned char *bitbuf;
//    jbyte *b_buffer = (*env)->GetByteArrayElements(env, buffer, 0);
//    pcmbuf = (int *) malloc(nInputSamples * sizeof(int));
//    bitbuf = (unsigned char *) malloc(nMaxOutputBytes * sizeof(unsigned char));
//    int nByteCount = 0;
//    unsigned int nBufferSize = (unsigned int) len / 2;
//    unsigned short *buf = (unsigned short *) b_buffer;
//    while (nByteCount < nBufferSize) {
//        int audioLength = nInputSamples;
//        if ((nByteCount + nInputSamples) >= nBufferSize) {
//            audioLength = nBufferSize - nByteCount;
//        }
//        int i;
//        for (i = 0; i < audioLength; i++) {//每次从实时的pcm音频队列中读出量化位数为8的pcm数据。
//            int s = ((int16_t *) buf + nByteCount)[i];
//            pcmbuf[i] = s << 8;//用8个二进制位来表示一个采样量化点（模数转换）
//        }
//        nByteCount += nInputSamples;
//        //利用FAAC进行编码，pcmbuf为转换后的pcm流数据，audioLength为调用faacEncOpen时得到的输入采样数，bitbuf为编码后的数据buff，nMaxOutputBytes为调用faacEncOpen时得到的最大输出字节数
//        int byteslen = faacEncEncode(audio_encode_handle, pcmbuf, audioLength,
//                                     bitbuf, nMaxOutputBytes);
//        if (byteslen < 1) {
//            continue;
//        }
//        add_aac_body(bitbuf, byteslen);//从bitbuf中得到编码后的aac数据流，放到数据队列
//    }
//    (*env)->ReleaseByteArrayElements(env, buffer, b_buffer, 0);
//    if (bitbuf)
//        free(bitbuf);
//    if (pcmbuf)
//        free(pcmbuf);
}