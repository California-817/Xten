# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
#添加汇编支持
enable_language(ASM)
# 项目名称和版本
project(Xten VERSION 1.0)

# 设置输出库路径为当前项目的 lib 目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 查找依赖库
find_package(yaml-cpp REQUIRED)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include  ${YAML_CPP_INCLUDE_DIRS})

# 源文件列表
file(GLOB SOURCES "src/*.cpp")

# 添加汇编源文件
file(GLOB ASM_SOURCES "include/fcontext/*.S")

# 构建动态库，包含 C++ 和汇编源文件
add_library(Xten SHARED ${SOURCES} ${ASM_SOURCES})

# 链接依赖的第三方库---无论是你要生成的是动态库还是静态库 都不需要连接第三方库
# 在生成可执行程序时，链接器的主要任务是解析所有符号引用，确保所有函数和变量的引用都能正确解析。这包括静态库和动态库中的符号。链接器会生成一个符号表，记录所有需要在运行时解析的符号。
# 可执行程序需要在运行时独立运行，因此必须解析所有符号引用，确保所有函数和变量的引用都能正确解析。这要求在生成可执行程序时，链接器必须解析所有依赖的库。
# 因为在生成动静态库[可执行程序]的时候并不会进行动态库符号的解析工作--->在程序启动后 加载动态库后 第一次调用到函数时才绑定地址[延迟绑定--提高程序响应速度]
# 静态库中的函数在编译时会被直接链接到可执行文件中[按需包含动态库的.o文件]，因此静态库中的函数调用不需要延迟绑定（Lazy Binding）
# 如果动态库 a 依赖另一个动态库 b，而可执行程序只链接了 a，那么在程序启动时，操作系统会自动加载 b。----递归查看依赖表并加载动态库
# 这是因为动态库的依赖关系在运行时由操作系统的动态加载器解析和加载。这种机制确保了所有必要的库都能被正确加载，即使它们没有被直接链接到可执行文件中。
# target_link_libraries(Xten PRIVATE  ${YAML_CPP_LIBRARIES} dl)
