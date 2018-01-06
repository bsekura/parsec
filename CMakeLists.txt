cmake_minimum_required(VERSION 3.0)
project(zuni CXX)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin")

include_directories(src)

add_executable(parsec
    src/json.cpp
    src/main.cpp)
    
target_compile_options(parsec PUBLIC -g -Wall -Wextra -pedantic
                                      -Wno-missing-braces -Wno-unused-parameter -Wno-c99-extensions
                                      -std=c++14 -fno-rtti -fno-exceptions) 
