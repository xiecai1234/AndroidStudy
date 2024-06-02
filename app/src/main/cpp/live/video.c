//
// Created by xiecaibao on 2024/5/31.
//
#include <malloc.h>
#include <string.h>
#include <jni.h>
#include "rtmp.h"
#include "x264.h"
#include "../include/utils/logger.h"
#include "../include/utils/common.h"

extern void add_rtmp_packet();
extern void add_264_sequence_header(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len);
extern void add_264_body(unsigned char *buf, int len);

//x264编码输入图像I420
x264_picture_t pic_in;
x264_picture_t pic_out;
//YUV个数
int y_len, u_len, v_len;
//x264编码处理器
x264_t *video_encode_handle;

/**
 *
参照源码example.c来写的
x264_param_default_preset 设置参数,要设置的太多了
x264_param_apply_profile 设置级别（档次）
x264_picture_alloc（x264_picture_t输入图像）初始化
x264_encoder_open 打开编码器
x264_encoder_encode 编码
x264_encoder_close( h ) 关闭编码器，释放资源*
 */
void setVideoOptions(JNIEnv *env, jobject jobj, jint width, jint height, jint bitrate, jint fps) {\
    LOGI("setVideoOptions");
    x264_param_t param;
    //x264_param_default_preset 设置  控制编码速度和延迟，ultrafast编码速度最快  zerolatency 0延迟，将延迟降到最低，且没有B帧
    //example.c-->base.c-->param_apply_preset
    x264_param_default_preset(&param, "ultrafast", "zerolatency");

//    YUV存储方式分为两大类：packed和planar。
//    packed：每个像素点的 Y、U、V 连续交叉存储。非常少用。
//    planar：先存储所有Y，紧接着存储所有U，最后是V。 planar又区分了以下几种格式： YUV444、 YUV422、YUV420。
//    YUV 4:2:0采样，每四个Y共用一组UV分量，最常用的就是YUV420。
//    YUV420又分两种类型：
//    YUV420P：三平面存储。数据组成为YYYYYYYYUUVV（如I420）或YYYYYYYYVVUU（如YV12）。
//    YUV420SP：两平面存储。分为两种类型YYYYYYYYUVUV（如NV12）或YYYYYYYYVUVU（如NV21）

    //编码输入的像素格式，I420 如果相机采集的不是I420，fireVideo里会转换为I420
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;

    y_len = width * height;
    u_len = y_len / 4;
    v_len = u_len;

    //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    //设定了恒定码率也不可能做到，会尽量控制在固定码率
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.i_bitrate = bitrate / 1000; //* 码率(比特率,单位Kbps)
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2; //瞬时最大码率
    // param.b_annexb不设置;               /* if set, place start codes (4 bytes) before NAL units,
    //码率控制，不通过timebase和timestamp，而是fps，省事很多  看头文件注释
    param.b_vfr_input = 0;

    param.i_fps_num = fps; //* 帧率分子
    param.i_fps_den = 1; //* 帧率分母
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;
    param.i_threads = 1;//并行编码线程数量（单线程），0默认为多线程

    //是否把SPS和PPS放入每一个关键帧 为了提高图像的纠错能力，前面的画面出错了，后面的不会再出错  fireVideo方法里根据这个设置来写
    //SPS Sequence Parameter Set 序列参数集，PPS Picture Parameter Set 图像参数集
    param.b_repeat_headers = 1;
    //设置Level级别为最高5.1
    param.i_level_idc = 51;
    //设置Profile档次为baseline级别，没有B帧
    x264_param_apply_profile(&param, "baseline");

    //x264_picture_t（输入图像）初始化
    x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height);
    pic_in.i_pts = 0;
    //打开编码器
    video_encode_handle = x264_encoder_open(&param);
    if (video_encode_handle) {
        LOGI("打开视频编码器成功");
    } else {
        throwNativeError(env, INIT_FAILED);
    }
}

void fireVideo(JNIEnv *env, jobject jobj, jbyteArray buffer) {
    LOGI("fireVideo");
    //视频数据由NV21转为I420  转换格式网上可查 TODO 尝试不转换看效果 数组操作
    //相机传过来的数据格式为NV21，x264_encoder_encode函数需要的数据格式为I420
    jbyte *nv21_buffer = (*env)->GetByteArrayElements(env, buffer, NULL);
    unsigned char *u = pic_in.img.plane[1];
    unsigned char *v = pic_in.img.plane[2];
    //nv21 4:2:0 Formats, 12 Bits per Pixel
    //nv21转yuv420p  y = w*h,u和v=w*h/4
    memcpy(pic_in.img.plane[0], nv21_buffer, y_len);
//    YUV420P：三平面存储。数据组成为YYYYYYYYUUVV（如I420，又叫YU12）或YYYYYYYYVVUU（如YV12）。
//    YUV420SP：两平面存储。分为两种类型YYYYYYYYUVUV（如NV12）或YYYYYYYYVUVU（如NV21）
    int i;
    for (i = 0; i < u_len; i++) {
        *(u + i) = *(nv21_buffer + y_len + i * 2 + 1);
        *(v + i) = *(nv21_buffer + y_len + i * 2);
    }

    //h264编码得到NALU数组
    x264_nal_t *nal = NULL; //NAL
    int n_nal = -1; //NALU的个数
    //进行h264编码，编码后的数据存放在nal中，nalu的个数为n_nal
    if (x264_encoder_encode(video_encode_handle, &nal, &n_nal, &pic_in, &pic_out) < 0) {
        LOGE("%s", "编码失败");
        return;
    }
    //使用rtmp协议将h264编码的视频数据发送给流媒体服务器
    //帧分为关键帧和普通帧，为了提高画面的纠错率，关键帧应包含SPS和PPS数据
    int sps_len, pps_len;
    unsigned char sps[100];
    unsigned char pps[100];
    memset(sps, 0, 100);
    memset(pps, 0, 100);
    pic_in.i_pts += 1; //顺序累加
    //遍历NALU数组，根据NALU的类型判断
    for (i = 0; i < n_nal; i++) {
        if (nal[i].i_type == NAL_SPS) {
            //复制SPS数据
            sps_len = nal[i].i_payload - 4;
            memcpy(sps, nal[i].p_payload + 4, sps_len); //不复制四字节起始码
            // TODO 需要add_264_sequence_header吗？
        } else if (nal[i].i_type == NAL_PPS) {
            //复制PPS数据
            pps_len = nal[i].i_payload - 4;
            memcpy(pps, nal[i].p_payload + 4, pps_len); //不复制四字节起始码

            //发送序列信息
            //h264关键帧会包含SPS和PPS数据
            add_264_sequence_header(pps, sps, pps_len, sps_len);
        } else {
            //发送帧信息
            add_264_body(nal[i].p_payload, nal[i].i_payload);
        }

    }

    (*env)->ReleaseByteArrayElements(env, buffer, nv21_buffer, 0);
}