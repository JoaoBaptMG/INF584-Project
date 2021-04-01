cmake_minimum_required(VERSION 3.12)

project(INF584Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("external" "src")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(glfw3 3.3 REQUIRED)

file(GLOB_RECURSE SRCS "src/*" "external/*" "resources/*")

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # taken from https://stackoverflow.com/a/31987079
    foreach(FILE ${SRCS})
        # Get the directory of the source file
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
        
        # Remove common directory prefix to make the group
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        
        # Make sure we are using windows slashes
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endif()

add_executable(INF584Project ${SRCS})
target_link_libraries(INF584Project Threads::Threads glfw ${CMAKE_DL_LIBS})

if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    target_compile_options(INF584Project PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>)
endif()
 