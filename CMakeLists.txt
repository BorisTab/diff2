cmake_minimum_required(VERSION 3.15)
project(diff2)

set(CMAKE_CXX_STANDARD 17)

add_executable(diff2 main.cpp operations.h tree.h fileRead.h)