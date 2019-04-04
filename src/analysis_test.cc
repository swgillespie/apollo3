#include "gtest/gtest.h"

#include "analysis.h"
#include "bitboard.h"
#include "position.h"

using apollo::Analysis;
using apollo::Bitboard;
using apollo::Position;
using apollo::Square;

TEST(AnalysisTest, DoublePawnSmoke) {
  Position p("8/6P1/2P5/4P3/2P2P2/PP1P2P1/P7/8 w - - 0 1");
  Analysis a(p);

  Bitboard doubled_pawns = a.DoubledPawns(apollo::kWhite);
  ASSERT_TRUE(doubled_pawns.Test(Square::A2));
  ASSERT_TRUE(doubled_pawns.Test(Square::A3));

  ASSERT_FALSE(doubled_pawns.Test(Square::B3));

  ASSERT_TRUE(doubled_pawns.Test(Square::C4));
  ASSERT_TRUE(doubled_pawns.Test(Square::C6));

  ASSERT_FALSE(doubled_pawns.Test(Square::D3));
  ASSERT_FALSE(doubled_pawns.Test(Square::E5));
  ASSERT_FALSE(doubled_pawns.Test(Square::F4));

  ASSERT_TRUE(doubled_pawns.Test(Square::G3));
  ASSERT_TRUE(doubled_pawns.Test(Square::G7));
}
