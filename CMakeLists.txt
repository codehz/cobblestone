cmake_minimum_required(VERSION 3.14)

project(cobblestone)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_C_FLAGS "-Wno-gnu-designator")

include_directories(custom)

add_subdirectory(deps/funchook EXCLUDE_FROM_ALL)
add_subdirectory(deps/qjs EXCLUDE_FROM_ALL)
add_subdirectory(deps/elf-parser EXCLUDE_FROM_ALL)
add_subdirectory(deps/wsrpc EXCLUDE_FROM_ALL)

add_subdirectory(builtin)
add_subdirectory(loader)

add_executable(stone src/main.cpp)
target_compile_options(stone PRIVATE -Wall)
set_property(TARGET stone PROPERTY CXX_STANDARD 17)