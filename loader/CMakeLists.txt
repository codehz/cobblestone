cmake_minimum_required(VERSION 3.14)

file(GLOB LOADER_SRC src/*.cpp)

add_library(loader SHARED ${LOADER_SRC})
target_link_libraries(loader funchook elf-parser)
target_include_directories(loader PRIVATE ../deps/headers/include ../custom)
target_include_directories(loader PUBLIC include)
set_property(TARGET loader PROPERTY CXX_STANDARD 20)
set_target_properties(loader PROPERTIES PREFIX "")
target_compile_options(loader PRIVATE -Wall)
