#include "gtest/gtest.h"

#include "move.h"

using apollo::Move;
using apollo::Square;

TEST(Move, Quiet) {
  Move m = Move::Quiet(Square::A3, Square::A4);
  ASSERT_TRUE(m.IsQuiet());
  ASSERT_EQ(Square::A3, m.Source());
  ASSERT_EQ(Square::A4, m.Destination());
}

TEST(Move, Capture) {
  Move m = Move::Capture(Square::A3, Square::A4);
  ASSERT_FALSE(m.IsQuiet());
  ASSERT_TRUE(m.IsCapture());
  ASSERT_EQ(Square::A3, m.Source());
  ASSERT_EQ(Square::A4, m.Destination());
}
