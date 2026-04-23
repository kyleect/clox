cmake_minimum_required(VERSION 3.22)

set(C_STANDARD 90)
set(C_STANDARD_REQUIRED ON)

project(clox LANGUAGES C)

set(CMAKE_C_FLAGS "-std=c99 -Wall -lreadline")

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.c)

add_executable(clox ${SRC_FILES})
# target_compile_definitions(clox PRIVATE DEBUG_TRACE_EXECUTION)

add_executable(clox-test ${SRC_FILES})
target_compile_definitions(clox-test PRIVATE DEBUG_PRINT_CODE)