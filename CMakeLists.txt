cmake_minimum_required(VERSION 3.4.1)

project(Demo2)

# 搜索当前目录以及子目录中所有的以.cpp结尾的文件，然后把它们保存到  source_files 变量中
file(GLOB_RECURSE source_files src/*.c)

include_directories(src)

# message(${source_files})
# message(${CMAKE_SOURCE_DIR})
# get_property(dirs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
# message(">>> include_dirs=${dirs}") #打印一下目录情况

add_executable(Demo ${source_files})