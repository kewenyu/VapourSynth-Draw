cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME draw)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

include_directories(src)
include_directories("C:/Program Files (x86)/VapourSynth/sdk/include/vapoursynth")

link_directories("C:/Program Files (x86)/VapourSynth/sdk/lib64")

add_library(${PROJECT_NAME} SHARED
        src/draw.cpp
        src/draw.h
        src/vapoursynth-draw.cpp)
