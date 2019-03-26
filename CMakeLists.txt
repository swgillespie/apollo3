cmake_minimum_required(VERSION 3.1.0)
project(apollo CXX)
include(FetchContent)
include(GoogleTest)
enable_testing()

option(APOLLO_USE_WERROR "Build with -Werror, which can cause build breaks" OFF)
option(APOLLO_CLANG_UBSAN "Build with undefined behavior sanitizers" OFF)
option(APOLLO_CLANG_ASAN "Build with address sanitizers" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)
message(STATUS "Current build type is ${CMAKE_BUILD_TYPE}")

set(APOLLO_MAJOR_VERSION 0)
set(APOLLO_MINOR_VERSION 1)
set(CMAKE_CXX_STANDARD 17)
set(GTEST_VERSION release-1.8.0)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        ${GTEST_VERSION}
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
endif(NOT googletest_POPULATED)

if(APOLLO_CLANG_UBSAN)
  message(STATUS "Building with undefined behavior sanitizers")
  add_compile_options(-fsanitize=ubsan)
endif(APOLLO_CLANG_UBSAN)

if(APOLLO_CLANG_ASAN)
  message(STATUS "Building with address sanitizers")
  add_compile_options(-fsanitize=address)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif(APOLLO_CLANG_ASAN)

if(APOLLO_USE_WERROR)
  message(STATUS "Building with warnings as errors")
  add_compile_options(-Werror)
endif(APOLLO_USE_WERROR)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Wno-unused-private-field)
add_compile_options(-pedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-g)
  add_compile_options(-DDEBUG)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")


add_subdirectory(${googletest_SOURCE_DIR}/googletest ${googletest_BINARY_DIR})
include_directories(SYSTEM ${googletest_SOURCE_DIR}/googletest/include ${googletest_SOURCE_DIR}/googletest)
include_directories(SYSTEM third_party)

add_subdirectory(src)
