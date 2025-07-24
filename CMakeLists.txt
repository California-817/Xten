# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
#添加汇编支持
enable_language(ASM)
# 项目名称和版本
project(Xten VERSION 1.0)

include(cmake/utils.cmake)

# 设置输出库路径为当前项目的 lib 目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 查找依赖库
find_package(yaml-cpp REQUIRED)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/src  ${YAML_CPP_INCLUDE_DIRS})

# 源文件列表
file(GLOB SOURCES src/*.cpp)
file(GLOB HTTP_SOURCES src/http/*.cpp)
file(GLOB WEBSOCKET_SOURCES src/websocket/*.cpp)
file(GLOB STREAMS_SOURCES src/streams/*.cpp)

# 合并所有源文件
list(APPEND SOURCES ${HTTP_SOURCES} ${WEBSOCKET_SOURCES} ${STREAMS_SOURCES})
#添加.rl文件的cpp文件
ragelmaker(src/http/http11_parser.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/http/httpclient_parser.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/http)

# 添加汇编源文件
file(GLOB ASM_SOURCES "src/fcontext/*.S")

# 构建动态库，包含 C++ 和汇编源文件
add_library(Xten SHARED ${SOURCES} ${ASM_SOURCES})
