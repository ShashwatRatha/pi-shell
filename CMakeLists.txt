cmake_minimum_required(VERSION 3.10)

project(pish VERSION 1.0)

file(GLOB SOURCES "src/*.c")
add_executable(pish ${SOURCES})

target_include_directories(pish PRIVATE $(CMAKE_SOURCE_DIR)/include)
