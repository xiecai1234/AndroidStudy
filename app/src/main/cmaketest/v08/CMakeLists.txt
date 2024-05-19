cmake_minimum_required(VERSION 3.0)
project(TEST)
set(HEAD_PATH ${PROJECT_SOURCE_DIR}/include)
set(LIB_PATH  ${PROJECT_SOURCE_DIR}/lib)
set(EXE_PATH  ${PROJECT_SOURCE_DIR}/bin)

set(LIB_NAME_ADD  add)
set(LIB_NAME_SUB  sub)

set(APP_NAME_ADD  app1)

# 指定源文件或者库对应的头文件路径
include_directories(${HEAD_PATH})

add_subdirectory(add)
add_subdirectory(sub)
add_subdirectory(test)