cmake_minimum_required(VERSION 3.27)
project(CrusherChess)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXE_LINKER_FLAGS "-static")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3") #to optimize current cpu: -march=native
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /fp:fast")
    endif()
endif()


add_executable(CrusherChess
        CrusherChess/src/CrusherChess.cpp
        CrusherChess/src/Debug.cpp
        CrusherChess/src/types.h
        CrusherChess/src/macros.h)

add_executable(TestExecutable
        CrusherChess/tests/test.cpp)