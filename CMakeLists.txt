cmake_minimum_required(VERSION 3.22)

set(C_STANDARD 90)
set(C_STANDARD_REQUIRED ON)

project(clox LANGUAGES C)

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.c)

# -DDEBUG_TRACE_EXECUTION for trace logging
# -DDEBUG_PRINT_CODE for disassembled bytecode
add_definitions()

add_executable(clox ${SRC_FILES})