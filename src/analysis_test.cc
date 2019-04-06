#include "gtest/gtest.h"

#include "analysis.h"
#include "bitboard.h"
#include "log.h"
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

TEST(AnalysisTest, BackwardPawnSmoke) {
  Position p("8/8/8/8/8/2P1P3/3P4/8 w - - 0 1");
  Analysis a(p);

  Bitboard isolated_pawns = a.BackwardPawns(apollo::kWhite);
  ASSERT_EQ(1, isolated_pawns.Count());
  ASSERT_TRUE(isolated_pawns.Test(Square::D2));
}

TEST(AnalysisTest, BackwardPawnBlackSmoke) {
  Position p("8/3p4/2p1p3/8/8/8/8/8 b - - 0 1");
  Analysis a(p);
  Bitboard backward_pawns = a.BackwardPawns(apollo::kBlack);
  ASSERT_EQ(1, backward_pawns.Count());
  ASSERT_TRUE(backward_pawns.Test(Square::D7));
}

TEST(AnalysisTest, IsolatedPawnSmoke) {
  Position p("8/8/8/8/8/3P1P2/6P1/8 w - - 0 1");
  Analysis a(p);

  Bitboard isolated_pawns = a.IsolatedPawns(apollo::kWhite);
  ASSERT_EQ(1, isolated_pawns.Count());
  ASSERT_TRUE(isolated_pawns.Test(Square::D3));
}
