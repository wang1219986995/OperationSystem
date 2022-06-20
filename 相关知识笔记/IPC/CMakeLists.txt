cmake_minimum_required(VERSION 3.10)
project(IPC)

set(CMAKE_CXX_STANDARD 11)

add_executable(IPC main.cpp)

add_executable(esm SEM/sem.cpp)


add_executable(fifo_read FIFO/read.cpp)
add_executable(fifo_write FIFO/write.cpp)
add_executable(pipe PIPE/pipe.cpp)
