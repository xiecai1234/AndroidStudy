cmake_minimum_required(VERSION 3.22.1)

project("media")

# 支持C++11标准和GNU扩展特性
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# 1. 定义so库和头文件所在目录，方面后面使用
set(ffmpeg_head_dir ${CMAKE_CURRENT_SOURCE_DIR}/include/ffmpeg)
set(ffmpeg_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
message(ffmpeg_head_dir: ${ffmpeg_head_dir})
message(ffmpeg_lib_dir: ${ffmpeg_lib_dir})
# 2. 添加头文件目录
include_directories(${ffmpeg_head_dir})

link_directories(${ffmpeg_lib_dir})

add_library(${PROJECT_NAME} SHARED
        media.cpp
)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${PROJECT_NAME}
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
)