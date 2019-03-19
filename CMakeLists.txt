cmake_minimum_required(VERSION 3.1.0)
project(apollo CXX)
include(FetchContent)
include(GoogleTest)
enable_testing()

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

add_subdirectory(${googletest_SOURCE_DIR}/googletest ${googletest_BINARY_DIR})
include_directories(SYSTEM ${googletest_SOURCE_DIR}/googletest/include ${googletest_SOURCE_DIR}/googletest)

add_subdirectory(src)
