cmake_minimum_required(VERSION 3.15.0)


project(Mem_DB VERSION 0.1.0)

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    if(UNIX)
        set	(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic")    
    endif(UNIX)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

add_subdirectory(src)

add_subdirectory(app)



