cmake_minimum_required(VERSION 3.23)
project(cluster C)

set(CMAKE_C_STANDARD 99)

add_executable(cluster cluster.c)

SET(GCC_COVERAGE_COMPILE_FLAGS "-DNDEBUG")
