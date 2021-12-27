cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX)
    set	(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic -Werror")    
endif(UNIX)

project(Mem_DB VERSION 0.1.0)


add_library(Mem_DB src/Mem_DB.cpp)

add_executable(kiv-cpp-sp-02 app/main.cpp)

include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)

