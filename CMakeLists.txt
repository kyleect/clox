cmake_minimum_required(VERSION 3.22)

set(C_STANDARD 90)
set(C_STANDARD_REQUIRED ON)

project(clox LANGUAGES C)

set(CMAKE_C_FLAGS "-std=c99 -Wall -lreadline")

set(VERSION_TXT ${CMAKE_SOURCE_DIR}/VERSION.txt)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
set(GENERATED_VERSION_C ${GENERATED_DIR}/version.c)

add_custom_command(
    OUTPUT ${GENERATED_VERSION_C}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
    COMMAND xxd -i ${VERSION_TXT} > ${GENERATED_VERSION_C}
    DEPENDS ${VERSION_TXT}
    COMMENT "Generating version.c from VERSION.txt"
)

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.c)

add_executable(clox ${SRC_FILES} ${GENERATED_VERSION_C})
# target_compile_definitions(clox PRIVATE DEBUG_TRACE_EXECUTION)

add_executable(clox-test ${SRC_FILES} ${GENERATED_VERSION_C})
target_compile_definitions(clox-test PRIVATE DEBUG_PRINT_CODE)