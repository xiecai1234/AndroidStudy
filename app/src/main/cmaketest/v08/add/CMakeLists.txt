cmake_minimum_required(VERSION 3.0)
project(${LIB_NAME_ADD})
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# 指定生成的add库放置的位置
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
# 生成add库
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})