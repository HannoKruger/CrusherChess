cmake_minimum_required(VERSION 3.27)
project(CrusherChess)

set(CMAKE_CXX_STANDARD 14)

add_executable(CrusherChess
        CrusherChess/CrusherChess/LinuxBuild/CrusherChess.cpp
        CrusherChess/CrusherChess/CrusherChess.cpp)
