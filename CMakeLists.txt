# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)

enable_testing()

project(clox LANGUAGES C)

# -----------------------------------------------------------------
# General compiler flags
# -----------------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-Wall")

# -----------------------------------------------------------------
# Version‑generation
# -----------------------------------------------------------------
set(VERSION_TXT ${CMAKE_SOURCE_DIR}/VERSION.txt)
set(VERSION_C   ${CMAKE_SOURCE_DIR}/src/version.c)   # <-- generated in‑place

# Custom command that creates/updates src/version.c
add_custom_command(
    OUTPUT  ${VERSION_C}
    COMMAND ${CMAKE_COMMAND} -E env bash ${CMAKE_SOURCE_DIR}/scripts/generate_version_c.sh ${VERSION_C}
    DEPENDS ${VERSION_TXT} ${CMAKE_SOURCE_DIR}/scripts/generate_version_c.sh
    COMMENT "Generating src/version.c from VERSION.txt"
)

add_custom_target(generate_version_c
    DEPENDS ${VERSION_C}
)

set_source_files_properties(${VERSION_C} PROPERTIES GENERATED TRUE)

# -----------------------------------------------------------------
# Source files
# -----------------------------------------------------------------
# Glob all .c files *except* the generated one – it will be added later.
file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.c)
list(FILTER SRC_FILES EXCLUDE REGEX ".*/version\\.c$")

# ---
# Test Source Files
# ---
file(GLOB UNIT_TEST_SRCS unit/*.c)

option(BUILD_TESTS "Build unit‑tests for project" OFF)

# -----------------------------------------------------------------
# Executables
# -----------------------------------------------------------------
# Add the generated file to the source list for each target.
add_executable(clox ${SRC_FILES} ${VERSION_C})
# target_compile_definitions(clox PRIVATE DEBUG_STRESS_GC)
# target_compile_definitions(clox PRIVATE DEBUG_TRACE_EXECUTION)
# target_compile_definitions(clox PRIVATE DEBUG_STRESS_GC)
# target_compile_definitions(clox PRIVATE DEBUG_LOG_GC)

add_executable(clox-test ${SRC_FILES} ${VERSION_C})
target_compile_definitions(clox-test PRIVATE DEBUG_PRINT_CODE DEBUG_STRESS_GC)

target_link_libraries(clox          PRIVATE readline m)
target_link_libraries(clox-test     PRIVATE readline m)

add_dependencies(clox generate_version_c)
add_dependencies(clox-test generate_version_c)

if (BUILD_TESTS)

# -----------------------------------------------------------------
# Tests
# -----------------------------------------------------------------

add_executable(clox-version-unit-test unit/version.c ${VERSION_C})
add_dependencies(clox-version-unit-test generate_version_c)
add_test(
    NAME   clox-version-unit-test
    COMMAND clox-version-unit-test
)

add_executable(clox-token-token-test unit/token.c src/token.c ${VERSION_C})
add_dependencies(clox-token-token-test generate_version_c)
add_test(
    NAME   clox-token-unit-test
    COMMAND clox-token-unit-test
)

add_executable(clox-value-unit-test unit/value.c src/value.c src/object.c src/hashtable.c src/chunk.c src/memory.c src/vm.c src/assert.c src/scanner.c src/compiler.c src/token.c src/native.c ${VERSION_C})
add_dependencies(clox-value-unit-test generate_version_c)
target_link_libraries(clox-value-unit-test     PRIVATE readline m)
add_test(
    NAME   clox-value-unit-test
    COMMAND clox-value-unit-test
)

endif()

# -----------------------------------------------------------------
# Coverage option
# -----------------------------------------------------------------
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")

    # GCC/Clang-compatible flags
    set(COVERAGE_COMPILE_FLAGS --coverage -O0 -g)
    set(COVERAGE_LINK_FLAGS    --coverage)

    target_compile_options(clox-test PRIVATE ${COVERAGE_COMPILE_FLAGS})
    target_link_options(clox-test PRIVATE ${COVERAGE_LINK_FLAGS})
endif()