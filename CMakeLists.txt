cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(a65_assembler)

# Build settings
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${SRC_DIR}/include")

# Main program
add_executable(a65)
target_sources(a65 PRIVATE
    ${SRC_DIR}/a65.cpp
    ${SRC_DIR}/createBuffer.cpp
    ${SRC_DIR}/lexer.cpp
    ${SRC_DIR}/token.cpp
)
target_compile_options(a65 PRIVATE
    -Wall
    -Wextra
    -O2
)
