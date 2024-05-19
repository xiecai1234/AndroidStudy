cmake_minimum_required(VERSION 3.0)
project(${LIB_NAME_ADD})
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# 指定源文件或者库对应的头文件路径
include_directories(${HEAD_PATH})
message(SRC_LIST:  ${SRC_LIST})
message(${HEAD_PATH})
# 指定生成的add库放置的位置
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
# 生成add库
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})