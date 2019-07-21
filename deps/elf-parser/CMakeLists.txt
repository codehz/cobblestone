cmake_minimum_required(VERSION 3.14)

add_library(elf-parser src/elf_parser.cpp)
target_include_directories(elf-parser PUBLIC include)