cmake_minimum_required(VERSION 3.23)

include(FetchContent)
project(redis)

SET (CMAKE_CXX_STANDARD 23)
SET (CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git 
    GIT_TAG 11.1.4
)
FetchContent_MakeAvailable(fmt)

set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)  

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(src)
add_subdirectory(tests)
