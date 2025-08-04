cmake_minimum_required(VERSION 3.10)

project(Mymodule)

set(CMAKE_CXX_STANDARD 17)

find_package(yaml-cpp REQUIRED)

include_directories(${YAML_CPP_INCLUDE_DIRS})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_library(Mymodule SHARED
    mymodule/mymodule.cpp
)
target_link_libraries(Mymodule 
                                ${YAML_CPP_LIBRARIES}
                                Xten
                                ssl
                                crypto
                                pthread
                                z
                                )
set_target_properties(Mymodule PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin/module
)

set_target_properties(Mymodule PROPERTIES
    OUTPUT_NAME "Mymodule"
)