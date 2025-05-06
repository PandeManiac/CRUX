cmake_minimum_required(VERSION 3.10...3.21)

project(CRUX LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wfloat-equal
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wmissing-declarations
    -Wdouble-promotion
    -Wformat=2
    -Wundef
    -Wcast-align
    -Wcast-qual
    -Wstrict-overflow=5
    -Wnull-dereference
    -Wredundant-decls
    -Wdisabled-optimization
    -Wno-newline-eof
    -Wstack-protector
    -Wpointer-arith
)

set(SANITIZERS
    -fsanitize=address
    -fsanitize=undefined
    -fsanitize=leak
    -fsanitize=null
    -fsanitize=bounds
    -fsanitize=float-divide-by-zero
    -fsanitize=integer
)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

find_program(CPPCHECK_EXE NAMES cppcheck)
if(CPPCHECK_EXE)
    set(CMAKE_C_CPPCHECK "${CPPCHECK_EXE};--enable=all;--inconclusive;--quiet")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(LTO_FLAGS "-flto")
    set(MORE_OPTIMIZATIONS 
        -march=native
        -funroll-loops
        -ftree-vectorize
        -fomit-frame-pointer
        -fno-exceptions
    )
endif()

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../src")
set(HEADER_DIR "${CMAKE_SOURCE_DIR}/../../header")

file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.c")
file(GLOB_RECURSE HEADER_FILES "${HEADER_DIR}/*.h")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${WARNING_FLAGS} ${SANITIZERS} -O0 -g -DNDEBUG>
    $<$<CONFIG:Release>:${WARNING_FLAGS} ${LTO_FLAGS} -O3 ${MORE_OPTIMIZATIONS}>
)

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
