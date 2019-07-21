cmake_minimum_required(VERSION 3.14)

add_library(quickjs src/quickjs src/cutils src/libregexp src/libunicode)
file(READ src/VERSION qjs_version)
string(STRIP "${qjs_version}" qjs_version)
target_compile_definitions(quickjs PRIVATE -D_GNU_SOURCE -DCONFIG_VERSION="${qjs_version}")
target_include_directories(quickjs INTERFACE include)