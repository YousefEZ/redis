cmake_minimum_required(VERSION 3.23)

project(redis)

SET (CMAKE_CXX_STANDARD 23)
SET (CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)
add_subdirectory(tests)
