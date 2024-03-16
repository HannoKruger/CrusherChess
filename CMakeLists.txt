cmake_minimum_required(VERSION 3.27)
project(CrusherChess)

set(CMAKE_CXX_STANDARD 23)

add_executable(CrusherChessLinux
        CrusherChess/CrusherChess/LinuxBuild/CrusherChess.cpp)

add_executable(CrusherChess
        CrusherChess/CrusherChess/CrusherChess.cpp)