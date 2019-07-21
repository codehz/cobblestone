cmake_minimum_required(VERSION 3.14)

file(GLOB mods CONFIGURE_DEPENDS mods/*)

foreach(mod_full ${mods})
  get_filename_component(mod ${mod_full} NAME)
  file(GLOB_RECURSE ${mod}_SRC
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    mods/${mod}/src/*.c
    mods/${mod}/src/*.cpp
    )
  add_library(builtin-mod-${mod} SHARED ${${mod}_SRC})
  set(${mod}_SRC_REL ${${mod}_SRC})
  list(TRANSFORM ${mod}_SRC_REL REPLACE ^mods/${mod}/src/ "")
  message("Found builtin mod: ${mod} - ${${mod}_SRC_REL}")
  target_include_directories(builtin-mod-${mod} PRIVATE ../loader/include)
  set_property(TARGET builtin-mod-${mod} PROPERTY CXX_STANDARD 20)
  set_target_properties(builtin-mod-${mod} PROPERTIES PREFIX ${mod})
  set_target_properties(builtin-mod-${mod} PROPERTIES SUFFIX "")
  set_target_properties(builtin-mod-${mod} PROPERTIES OUTPUT_NAME ".mod")
  target_compile_options(builtin-mod-${mod} PRIVATE -Wall)
  if(EXISTS ${mod_full}/include)
    target_include_directories(builtin-mod-${mod} PUBLIC ${mod_full}/include)
    file(GLOB_RECURSE ${mod}_HEADERS
      RELATIVE ${mod_full}/include
      LIST_DIRECTORIES false
      ${mod_full}/include/*.h ${mod_full}/include/*.hpp)
    message("Headers for builtin-mod-${mod}: ${${mod}_HEADERS}")
  endif()
  if(EXISTS ${mod_full}/deps.txt)
    file(READ ${mod_full}/deps.txt deps)
    string(STRIP "${deps}" deps)
    message("Dependencies for builtin-mod-${mod}: ${deps}")
    target_link_libraries(builtin-mod-${mod} PUBLIC "${deps}")
  endif()
endforeach(mod_full ${mods})
