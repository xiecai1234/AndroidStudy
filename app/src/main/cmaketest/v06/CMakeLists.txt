cmake_minimum_required(VERSION 3.0)
project(TEST)
set(HEAD_PATH ${PROJECT_SOURCE_DIR}/include)
set(LIB_PATH  ${PROJECT_SOURCE_DIR}/lib)
set(EXE_PATH  ${PROJECT_SOURCE_DIR}/bin)

set(LIB_NAME_ADD  add)
set(LIB_NAME_SUB  sub)

set(APP_NAME_ADD  app1)
set(APP_NAME_SUB  app2)

add_subdirectory(add)
add_subdirectory(sub)
add_subdirectory(test1)
add_subdirectory(test2)