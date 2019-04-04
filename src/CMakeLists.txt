set(APOLLO_LIB_SOURCES
  analysis.cc
  movegen.cc
  attacks.cc
  position.cc
  evaluators/shannon_evaluator.cc
)

set(APOLLO_SOURCES
  main_perft.cc
  main.cc
)

set(APOLLO_TEST_SOURCES
  analysis_test.cc
  position_test.cc
  move_test.cc
  bitboard_test.cc
  movegen_test.cc
  perft_test.cc
)

add_library(apollo ${APOLLO_LIB_SOURCES})

add_executable(apollo3 ${APOLLO_SOURCES})
target_link_libraries(apollo3 apollo)

add_executable(apollo_test ${APOLLO_TEST_SOURCES})
target_link_libraries(apollo_test apollo gtest gtest_main)
gtest_discover_tests(apollo_test)
