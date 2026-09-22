cmake_minimum_required(VERSION 4.2)
project(JeniLang LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor/mini.c)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)

add_executable(JeniCompiler ${SOURCES})
target_include_directories(JeniCompiler PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/vendor/mini.c/include
)
target_link_libraries(JeniCompiler PRIVATE mini.c)
set_target_properties(JeniCompiler PROPERTIES
  RUNTIME_OUTPUT_NAME "jeni"
)
