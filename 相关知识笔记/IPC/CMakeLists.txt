cmake_minimum_required(VERSION 3.10)
project(IPC)

set(CMAKE_CXX_STANDARD 11)

add_executable(IPC main.cpp)

add_executable(esm SEM/sem.cpp)