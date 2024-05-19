cmake_minimum_required(VERSION 3.0)
project(${LIB_NAME_SUB})
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# 指定生成的sub库放置的位置
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
# 生成sub库
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
# 指定sub库要链接的库（add库）的路径
link_directories(${LIB_PATH})
# 指定sub库要链接的库（add库）
target_link_libraries(${PROJECT_NAME} ${LIB_NAME_ADD})