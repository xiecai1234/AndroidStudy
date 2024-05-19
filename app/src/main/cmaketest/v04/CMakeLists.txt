cmake_minimum_required(VERSION 3.0)
project(TEST)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
# 指定源文件或者动态库对应的头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 指定生成的calc动态库放置的位置
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 生成calc动态库
add_library(calc SHARED ${SRC_LIST})
# 指定要链接的动态库的路径
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 添加并生成一个可执行程序
add_executable(app main.cpp)
# 指定要链接的动态库
target_link_libraries(app calc)
