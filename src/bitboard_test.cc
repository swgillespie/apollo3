#include <vector>
#include "gtest/gtest.h"

#include "bitboard.h"

using apollo::Bitboard;
using apollo::Square;

TEST(Bitboard, Smoke) {
  Bitboard b;
  ASSERT_TRUE(b.Empty());
  b.Set(Square::A1);
  ASSERT_FALSE(b.Empty());
  ASSERT_TRUE(b.Test(Square::A1));
}

TEST(Bitboard, Iter) {
  Bitboard b;
  b.Set(Square::A1);
  b.Set(Square::B4);
  b.Set(Square::B5);
  std::vector<Square> squares;
  for (auto it = b.Iterator(); it.HasNext();) {
    squares.push_back(it.Next());
  }
  ASSERT_TRUE(std::find(squares.cbegin(), squares.cend(), Square::A1) !=
              squares.cend());
  ASSERT_TRUE(std::find(squares.cbegin(), squares.cend(), Square::B4) !=
              squares.cend());
  ASSERT_TRUE(std::find(squares.cbegin(), squares.cend(), Square::B5) !=
              squares.cend());
}
