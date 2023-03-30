cmake_minimum_required(VERSION 3.4.1)

# 指定 c++ 标准
set(CMAKE_CXX_STANDARD 11)

# 指定 cmake 生成文件的目录
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)

project(caspp-emulater)

# 搜索当前目录以及子目录中所有的以.cpp结尾的文件，然后把它们保存到  source_files 变量中
file(GLOB_RECURSE source_files src/*.c)

include_directories(src)

# 构建 emulater
add_executable(emulater ${source_files})

# 搜索目录下的所有.cpp文件
aux_source_directory(src/linker linker_source_files)

# 搜索目录下的所有.cpp文件
aux_source_directory(src/common common_source_files)

# 构建 linker
add_executable(linker ${linker_source_files} ${common_source_files})