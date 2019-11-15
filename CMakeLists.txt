cmake_minimum_required(VERSION 3.14)
project(pika_akinator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER g++-8)

include_directories(.)
include_directories(headers)

add_executable(pika_akinator
               main.cpp
               headers/Akinator.h
               sources/Akinator.cpp
               headers/AkinatorTree.h
               sources/AkinatorTree.cpp)
target_link_libraries(pika_akinator stdc++fs)