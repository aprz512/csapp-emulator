cmake_minimum_required(VERSION 3.4.1)

# 指定 c++ 标准
set(CMAKE_CXX_STANDARD 11)

project(caspp-emulater)

# 搜索当前目录以及子目录中所有的以.cpp结尾的文件，然后把它们保存到  source_files 变量中
file(GLOB_RECURSE source_files src/*.c)

include_directories(src)

add_executable(emulater ${source_files})