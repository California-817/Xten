cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)

enable_language(ASM)

project(Xten VERSION 1.0.0)

include(cmake/utils.cmake)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -Og -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated  -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -Og -fPIC -ggdb -std=c11 -Wall -Wno-deprecated  -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(FIBER_TYPE "FIBER_FCONTEXT" CACHE STRING "Fiber implementation type")
else()
    set(FIBER_TYPE "FIBER_UCONTEXT" CACHE STRING "Fiber implementation type")
endif()

set_property(CACHE FIBER_TYPE PROPERTY STRINGS "FIBER_UCONTEXT" "FIBER_FCONTEXT" "FIBER_COCTX")

if(NOT FIBER_TYPE STREQUAL "FIBER_UCONTEXT" AND NOT FIBER_TYPE STREQUAL "FIBER_FCONTEXT"
                                            AND NOT FIBER_TYPE STREQUAL "FIBER_COCTX")
    message(FATAL_ERROR "FIBER_TYPE must be either FIBER_UCONTEXT or FIBER_FCONTEXT or FIBER_COCTX, but got: ${FIBER_TYPE}")
endif()

#no multiple taskqueue , some bug 
set(OPTIMIZE "OFF" CACHE STRING "Whether open scheduler queue optimize")
set_property(CACHE OPTIMIZE PROPERTY STRINGS "ON" "OFF")
if(NOT OPTIMIZE STREQUAL "ON" AND NOT OPTIMIZE STREQUAL "OFF")
    message(FATAL_ERROR "optimize must be ON or OFF , but got: ${OPTIMIZE}")
endif()

add_compile_definitions(OPTIMIZE=${OPTIMIZE})
add_compile_definitions(FIBER_TYPE=${FIBER_TYPE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

find_package(yaml-cpp REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/src 
                    ${YAML_CPP_INCLUDE_DIRS}
)
include_directories("/usr/local/include")

link_directories("/usr/local/lib" "/usr/local/lib64"
		 "/usr/lib/aarch64-linux-gnu"		
)

file(GLOB SOURCES src/*.cpp)
file(GLOB HTTP_SOURCES src/http/*.cpp)
file(GLOB WEBSOCKET_SOURCES src/websocket/*.cpp)
file(GLOB STREAMS_SOURCES src/streams/*.cpp)
file(GLOB ROCK_SOURCES src/rock/*.cpp)
file(GLOB SYSTEM_SOURCES src/system/*.cpp)
file(GLOB MODULE_SOURCES src/module/*.cpp)
file(GLOB SERVLETS_SOURCES src/http/servlets/*.cpp)
file(GLOB DB_SOURCES src/db/*.cpp)
file(GLOB EMAIL_SOURCES src/email/*.cpp)
file(GLOB XFTP_SOURCES src/xftp/*.cpp)
file(GLOB KCP_SOURCES src/kcp/*.cpp src/kcp/third_part/*.c)

file(GLOB XORM_SOURCES src/orm/*.cpp)

list(APPEND SOURCES 
                    ${HTTP_SOURCES} 
                    ${WEBSOCKET_SOURCES} 
                    ${STREAMS_SOURCES}
                    ${ROCK_SOURCES}
                    ${SYSTEM_SOURCES}
                    ${MODULE_SOURCES}
                    ${SERVLETS_SOURCES}
                    ${DB_SOURCES}
                    ${EMAIL_SOURCES}
                    ${XFTP_SOURCES}
                    ${KCP_SOURCES}
)
list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/src/system/main.cpp)

ragelmaker(src/http/http11_parser.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/http/httpclient_parser.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/http)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "Building for x86_64 platform")
    
    file(GLOB ASM_SOURCES "src/fcontext/*.S")
    list(APPEND ASM_SOURCES "src/libco/coctx_Swap.S")
    list(APPEND SOURCES "src/libco/coctx.cpp")

    add_library(Xten STATIC ${SOURCES} ${ASM_SOURCES})
else()
    message(STATUS "Building for arm or other platform")

    add_library(Xten STATIC ${SOURCES})
endif()



set_target_properties(Xten PROPERTIES
                        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_executable(XtenFrame src/system/main.cpp)
add_executable(xorm ${XORM_SOURCES})
set_target_properties(xorm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/tools
)

add_executable(XftpClient test/xftp_client.cpp)
add_executable(rockClient test/test_rock.cpp)
add_executable(kcpClient test/test_kcp.cpp)


set(COMMON_LIBS    
                ssl
                crypto
                pthread
                dl
                z
                event
                hiredis
                mysqlclient
                jsoncpp
                tinyxml2
)

find_library(HIREDIS_CLUSTER_LIB
    NAMES hiredis_vip hiredis_cluster hiredis-vip
    PATHS /usr/local/lib /usr/lib /usr/lib64 /usr/local/lib64
)
if(HIREDIS_CLUSTER_LIB)
    message(STATUS "Found hiredis cluster lib: ${HIREDIS_CLUSTER_LIB}")
    list(APPEND COMMON_LIBS ${HIREDIS_CLUSTER_LIB})
else()
    message(STATUS "hiredis cluster library not found during CMake configure. If you need Redis Cluster support, 
                        please install hiredis-vip / hiredis-cluster (providing redisCluster* symbols) and re-run CMake.")
endif()

target_link_libraries(XtenFrame
                    Xten
                    ${YAML_CPP_LIBRARIES}
                    ${COMMON_LIBS}
)
target_link_libraries(xorm 
                    Xten
                    ${YAML_CPP_LIBRARIES}
                    ${COMMON_LIBS}
)

macro(test_example name)
target_link_libraries(${name}
        Xten
        ${YAML_CPP_LIBRARIES}
        ${COMMON_LIBS}
    )
endmacro()

test_example(XftpClient)
test_example(kcpClient)
test_example(rockClient)

