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

# 搜索目录下的所有.cpp文件
aux_source_directory(src/algorithm algorithm_source_files)

# 构建 linker
add_executable(linker ${linker_source_files} ${common_source_files} ${algorithm_source_files})

# 构建 cache
file(GLOB cache_src_list src/memory/dram.c src/cpu/sram.c src/test/cache.c src/common/convert.c src/common/log.c)
add_executable(cache ${cache_src_list})

# 非缓存测试构建，请注释掉改行
# add_definitions(-DCACHE_TEST)

# 构建 mesi
file(GLOB mesi_src_list src/cpu/mesi.c)
add_executable(mesi ${mesi_src_list})

# 构建 false sharing
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
file(GLOB false_sharing_src_list src/test/falsesharing.c)
add_executable(false_sharing ${false_sharing_src_list})