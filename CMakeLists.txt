# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 项目名称和版本
project(Xten VERSION 1.0)

# 设置输出库路径为当前项目的 lib 目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件列表
file(GLOB SOURCES "src/*.cpp")

# 构建动态库
add_library(Xten SHARED ${SOURCES})