cmake_minimum_required(VERSION 3.16)
project(FossRedder)

set(CMAKE_CXX_STANDARD 17)

add_executable(fossredder src/main.cpp)

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)