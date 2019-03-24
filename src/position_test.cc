#include "gtest/gtest.h"

#include "move.h"
#include "position.h"

using apollo::Move;
using apollo::PieceKind;
using apollo::Position;
using apollo::Square;

TEST(PositionTest, BasicMovement) {
  Position p("8/8/8/8/8/4P3/8/8 w - -");
  auto e3 = p.PieceAt(Square::E3);
  ASSERT_TRUE(e3.has_value());
  ASSERT_EQ(PieceKind::kPawn, e3->kind());
  ASSERT_EQ(apollo::kWhite, e3->color());

  p.MakeMove(Move::Quiet(Square::E3, Square::E4));
  ASSERT_FALSE(p.PieceAt(Square::E3).has_value());
  ASSERT_TRUE(p.PieceAt(Square::E4).has_value());
  ASSERT_EQ(apollo::kBlack, p.SideToMove());
  ASSERT_FALSE(p.EnPassantSquare().has_value());
}

TEST(PositionTest, PawnHalfmoveClock) {
  Position p("8/8/8/8/8/4P3/8/8 w - - 5 6");
  p.MakeMove(Move::Quiet(Square::E3, Square::E4));
  ASSERT_EQ(0, p.HalfmoveClock());
}

TEST(PositionTest, BasicUnmake) {
  Position p("8/8/8/8/8/4P3/8/8 w - - 5 6");
  p.MakeMove(Move::Quiet(Square::E3, Square::E4));
  ASSERT_FALSE(p.PieceAt(Square::E3).has_value());
  ASSERT_TRUE(p.PieceAt(Square::E4).has_value());
  // White moved a pawn, so the halfmove clock resets.
  ASSERT_EQ(0, p.HalfmoveClock());

  p.UnmakeMove();
  ASSERT_TRUE(p.PieceAt(Square::E3).has_value());
  ASSERT_FALSE(p.PieceAt(Square::E4).has_value());

  // Unmake resets the halfmove and fullmove clocks to their
  // value before the move.
  ASSERT_EQ(5, p.HalfmoveClock());
  ASSERT_EQ(6, p.FullmoveClock());
}

TEST(PositionTest, FenParseHalfmove) {
  Position p("8/8/8/8/8/4P3/8/8 w - - 5 6");
  ASSERT_EQ(5, p.HalfmoveClock());
  ASSERT_EQ(6, p.FullmoveClock());
}
