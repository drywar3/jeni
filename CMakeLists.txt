cmake_minimum_required(VERSION 4.2)
project(JeniLang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor/mini.c)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cc)

add_executable(JeniCompiler ${SOURCES})
target_include_directories(JeniCompiler PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/vendor/mini.c/include
)
target_compile_options(JeniCompiler PRIVATE
  -Wno-c99-designator
  -ftime-trace)
target_link_libraries(JeniCompiler PRIVATE mini.c)
set_target_properties(JeniCompiler PROPERTIES
  RUNTIME_OUTPUT_NAME "jeni"
)
