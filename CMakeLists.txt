cmake_minimum_required(VERSION 4.2 FATAL_ERROR)
project(a65_assembler)

# Primarily for opcode array designation
set(CMAKE_CXX_STANDARD 20)

# Build settings
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${SRC_DIR}/include")

# Main program
add_executable(a65)
target_sources(a65 PRIVATE
    ${SRC_DIR}/a65.cpp
    ${SRC_DIR}/createBuffer.cpp
    ${SRC_DIR}/lexer.cpp
    ${SRC_DIR}/parser.cpp
    ${SRC_DIR}/assembler.cpp
    ${SRC_DIR}/token.cpp
    ${SRC_DIR}/type_printers.cpp
    ${SRC_DIR}/diagnostics.cpp
)
target_compile_options(a65 PRIVATE
    -Wall
    -Wextra
    -O2
)
