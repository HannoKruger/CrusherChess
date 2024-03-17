cmake_minimum_required(VERSION 3.27)
project(CrusherChess)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(CrusherChessLinux
        CrusherChess/CrusherChess/LinuxBuild/CrusherChess.cpp)

add_executable(CrusherChess
        CrusherChess/CrusherChess/CrusherChess.cpp)