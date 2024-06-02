cmake_minimum_required(VERSION 3.22.1)

project("myffmpeg")

# 支持C++11标准和GNU扩展特性
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# 1. 定义头文件所在目录，方便后面使用
set(ffmpeg_head_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/ffmpeg)
set(yuv_head_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/libyuv)
set(x264_head_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/x264)
set(faac_head_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/faac)
set(rtmp_head_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/rtmpdump)

#不打包，方便查看源码用
set(x264_all_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/source_code/x264-master)
# 1. 定义库文件所在目录，方便后面使用
set(lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

# 2. 添加头文件目录
include_directories(${ffmpeg_head_dir})
include_directories(${yuv_head_dir})
include_directories(${x264_head_dir})
include_directories(${faac_head_dir})
include_directories(${rtmp_head_dir})
#不打包，方便查看源码用
include_directories(${x264_all_src_dir})
# 3. 指定待链接的库文件所在目录
link_directories(${lib_dir})

# 查找某个路径下的所有源文件
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/live/*.c)
add_library(dn_live SHARED ${SRC_LIST})
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(dn_live
        # List libraries link to the target library
        android
        log
        avutil-54
        swresample-1
        avcodec-56
        avformat-56
        swscale-3
        postproc-53
        avfilter-5
        avdevice-56
        yuv
        jnigraphics
        x264
        faac
        rtmp
)
