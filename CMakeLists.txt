cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -flto")

project(arc)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)    
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(rlepack rlepack.cpp)
add_executable(rleunpack rleunpack.cpp)
add_executable(huffmanpack huffmanpack.cpp)
add_executable(huffmanunpack huffmanunpack.cpp)
add_executable(mtfpack mtfpack.cpp)
add_executable(mtfunpack mtfunpack.cpp)
add_executable(bwtpack bwtpack.cpp)
add_executable(bwtunpack bwtunpack.cpp)