cmake_minimum_required(VERSION 3.14)
project(algprog C)

set(CMAKE_C_STANDARD 99)

add_executable(algprog main.c)
target_link_libraries(algprog m)
