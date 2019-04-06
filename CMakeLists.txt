cmake_minimum_required(VERSION 3.9) # CMP0069 NEW
project(apollo CXX)
include(FetchContent)
include(GoogleTest)
enable_testing()

option(APOLLO_USE_WERROR "Build with -Werror, which can cause build breaks" OFF)
option(APOLLO_CLANG_UBSAN "Build with undefined behavior sanitizers" OFF)
option(APOLLO_CLANG_ASAN "Build with address sanitizers" OFF)
option(APOLLO_USE_LTO "Build with Link-Time Optimiation")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)
message(STATUS "Current build type is ${CMAKE_BUILD_TYPE}")

set(APOLLO_MAJOR_VERSION 0)
set(APOLLO_MINOR_VERSION 1)
set(APOLLO_PATCH_VERSION 0)

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

find_package(Doxygen)
if(DOXYGEN_FOUND)
  message(STATUS "Doxygen found, building documentation")

  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen")
endif(DOXYGEN_FOUND)


if(APOLLO_CLANG_UBSAN)
  message(STATUS "Building with undefined behavior sanitizers")
  add_compile_options(-fsanitize=undefined)
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
include_directories(src)

add_subdirectory(src)

if (APOLLO_USE_LTO)
  set_property(TARGET apollo3 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif(APOLLO_USE_LTO)