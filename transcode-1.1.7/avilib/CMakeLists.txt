cmake_minimum_required(VERSION 3.22.1)

project("transcode")

get_filename_component(_transcode_path_lib_tanscode ${PROJECT_SOURCE_DIR} ABSOLUTE)
message("_transcode_path_lib_tanscode : ${_transcode_path_lib_tanscode}")
set(include_header_list ${_transcode_path_lib_tanscode})

#aux_source_directory(${_transcode_path_lib_tanscode} SRC_LIST)
#MESSAGE("tiandihui ${SRC_LIST}")

# 编译
add_library(wavlib_shared SHARED
        wavlib.c platform_posix.c)

#foreach (_header_path ${include_header_list})
#    include_directories(${_header_path})
#endforeach ()
include_directories(${include_header_list})