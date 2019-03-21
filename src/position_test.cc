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
  ASSERT_EQ(PieceKind::kPawn, e3->Kind());
  ASSERT_EQ(apollo::kWhite, e3->Color());

  p.MakeMove(Move::Quiet(Square::E3, Square::E4));
  ASSERT_FALSE(p.PieceAt(Square::E3).has_value());
  ASSERT_TRUE(p.PieceAt(Square::E4).has_value());
  ASSERT_EQ(apollo::kBlack, p.SideToMove());
  ASSERT_FALSE(p.EnPassantSquare().has_value());
}

TEST(PositionTest, PawnHalfmoveClock) {
  Position p("8/8/8/8/8/4P3/8/8 w - -");
  p.MakeMove(Move::Quiet(Square::E3, Square::E4));
  ASSERT_EQ(0, p.HalfmoveClock());
}

TEST(PositionTest, BasicUnmake) {
  Position p("8/8/8/8/8/4P3/8/8 w - -");
  p.MakeMove(Move::Quiet(Square::E3, Square::E4));
  ASSERT_FALSE(p.PieceAt(Square::E3).has_value());
  ASSERT_TRUE(p.PieceAt(Square::E4).has_value());

  p.UnmakeMove();
  ASSERT_TRUE(p.PieceAt(Square::E3).has_value());
  ASSERT_FALSE(p.PieceAt(Square::E4).has_value());
}

TEST(PositionDeathTest, InvalidMoveSourceSquare) {
  Position p("8/8/8/8/8/4P3/8/8 w - -");
  Move m = Move::Quiet(Square::A4, Square::A5);
  ASSERT_DEATH(p.MakeMove(m), "no piece at move source square");
}
