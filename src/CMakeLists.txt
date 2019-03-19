set(APOLLO_LIB_SOURCES
  position.cc
)

set(APOLLO_SOURCES
  main.cc
)

set(APOLLO_TEST_SOURCES
  move_test.cc
  bitboard_test.cc
)

add_library(apollo ${APOLLO_LIB_SOURCES})

add_executable(apollo_cli ${APOLLO_SOURCES})
target_link_libraries(apollo_cli apollo)

add_executable(apollo_test ${APOLLO_TEST_SOURCES})
target_link_libraries(apollo_test apollo gtest gtest_main)
gtest_discover_tests(apollo_test)
