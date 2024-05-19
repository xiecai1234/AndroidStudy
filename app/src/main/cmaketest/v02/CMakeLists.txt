cmake_minimum_required(VERSION 3.0)
project(CALC)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
message("source path: ${SRC}")
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(app ${SRC})
