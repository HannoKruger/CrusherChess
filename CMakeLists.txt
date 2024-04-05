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
        CrusherChess/src/macros.h
        CrusherChess/src/random.h
        CrusherChess/src/uci.h
        CrusherChess/src/uci.cpp
        CrusherChess/src/movegen.h
        CrusherChess/src/movegen.cpp
        CrusherChess/src/magics.cpp
        CrusherChess/src/magics.h
        CrusherChess/src/bitoperations.cpp
        CrusherChess/src/bitoperations.h
        CrusherChess/src/zobristKeys.h
        CrusherChess/src/zobristKeys.cpp
        CrusherChess/src/transposition.cpp
        CrusherChess/src/transposition.h
        CrusherChess/src/typedefs.h
        CrusherChess/src/hash.h
        CrusherChess/src/moves.h
        CrusherChess/src/types.cpp
        CrusherChess/src/search.cpp
        CrusherChess/src/search.h
        CrusherChess/src/evaluation.cpp
        CrusherChess/src/evaluation.h
        CrusherChess/src/perft.cpp
        CrusherChess/src/perft.h
        CrusherChess/src/simpleSearch.cpp
        CrusherChess/src/simpleSearch.h
)

add_executable(TestExecutable
        CrusherChess/tests/test.cpp)