cmake_minimum_required(VERSION 3.0)
project(${APP_NAME_ADD})
# 指定源文件或者库对应的头文件路径
include_directories(${HEAD_PATH})

# 指定要链接的库的路径
link_directories(${LIB_PATH})
# 指定可执行程序存放路径
set(EXECUTABLE_OUTPUT_PATH ${EXE_PATH})
# 生成一个可执行程序
add_executable(${PROJECT_NAME} test1.cpp)
# 指定要链接的库
target_link_libraries(${PROJECT_NAME} ${LIB_NAME_ADD})