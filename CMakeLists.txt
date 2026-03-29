cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -flto")

project(arc)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)    
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(arc main.cpp)
add_executable(converter converter.cpp)
add_executable(test test.cpp)