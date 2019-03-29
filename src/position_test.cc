#include "gtest/gtest.h"

#include "log.h"
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

TEST(PositionTest, BasicCapture) {
  Position p("rnbqkbnr/ppppp1pp/8/5p2/8/6N1/PPPPPPPP/RNBQKB1R w KQkq - 5 6");
  p.MakeMove(Move::Capture(Square::G3, Square::F5));

  // There's a white knight at F5.
  auto knight = p.PieceAt(Square::F5);
  ASSERT_TRUE(knight.has_value());
  ASSERT_EQ(apollo::kWhite, knight->color());
  ASSERT_EQ(apollo::kKnight, knight->kind());

  // There's not a white knight at G3
  ASSERT_FALSE(p.PieceAt(Square::G3).has_value());

  // The halfmove clock reset due to a capture.
  ASSERT_EQ(0, p.HalfmoveClock());
}

TEST(PositionTest, BasicUnmakeCapture) {
  Position p("rnbqkbnr/ppppp1pp/8/5p2/8/6N1/PPPPPPPP/RNBQKB1R w KQkq - 5 6");
  p.MakeMove(Move::Capture(Square::G3, Square::F5));
  p.UnmakeMove();

  // The white knight has been restored to G3
  auto knight = p.PieceAt(Square::G3);
  ASSERT_TRUE(knight.has_value());
  ASSERT_EQ(apollo::kWhite, knight->color());
  ASSERT_EQ(apollo::kKnight, knight->kind());

  // The black pawn has been restored to F5
  auto pawn = p.PieceAt(Square::F5);
  ASSERT_TRUE(pawn.has_value());
  ASSERT_EQ(apollo::kBlack, pawn->color());
  ASSERT_EQ(apollo::kPawn, pawn->kind());

  // The halfmove clock has been reset to 5
  ASSERT_EQ(5, p.HalfmoveClock());
}

TEST(PositionTest, UnmakeKingsideCastle) {
  Position p("8/8/8/8/8/8/8/4K2R w KQkq - 0 1");
  p.MakeMove(Move::KingsideCastle(Square::E1, Square::G1));

  auto king = p.PieceAt(Square::G1);
  ASSERT_TRUE(king.has_value());
  ASSERT_EQ(apollo::kWhite, king->color());
  ASSERT_EQ(apollo::kKing, king->kind());

  auto rook = p.PieceAt(Square::F1);
  ASSERT_TRUE(rook.has_value());
  ASSERT_EQ(apollo::kWhite, rook->color());
  ASSERT_EQ(apollo::kRook, rook->kind());

  p.UnmakeMove();
  ASSERT_TRUE(p.CanCastleKingside(apollo::kWhite));
  auto prev_king = p.PieceAt(Square::E1);
  ASSERT_TRUE(prev_king.has_value());
  ASSERT_EQ(apollo::kWhite, prev_king->color());
  ASSERT_EQ(apollo::kKing, prev_king->kind());

  auto prev_rook = p.PieceAt(Square::H1);
  ASSERT_TRUE(prev_rook.has_value());
  ASSERT_EQ(apollo::kWhite, prev_rook->color());
  ASSERT_EQ(apollo::kRook, prev_rook->kind());

  ASSERT_FALSE(p.PieceAt(Square::G1).has_value());
  ASSERT_FALSE(p.PieceAt(Square::F1).has_value());
}

TEST(PositionTest, EnPassant) {
  Position p("8/8/3Pp3/8/8/8/8/8 w - e7 0 1");
  ASSERT_EQ(Square::E7, *p.EnPassantSquare());

  p.MakeMove(Move::EnPassant(Square::D6, Square::E7));
  auto white_pawn = p.PieceAt(Square::E7);
  ASSERT_TRUE(white_pawn.has_value());
  ASSERT_EQ(apollo::kWhite, white_pawn->color());
  ASSERT_EQ(apollo::kPawn, white_pawn->kind());
  ASSERT_FALSE(p.PieceAt(Square::E6).has_value());
  ASSERT_FALSE(p.EnPassantSquare().has_value());

  p.UnmakeMove();
  auto prev_white_pawn = p.PieceAt(Square::D6);
  ASSERT_TRUE(prev_white_pawn.has_value());
  ASSERT_EQ(apollo::kWhite, prev_white_pawn->color());
  ASSERT_EQ(apollo::kPawn, prev_white_pawn->kind());

  auto black_pawn = p.PieceAt(Square::E6);
  ASSERT_TRUE(black_pawn.has_value());
  ASSERT_EQ(apollo::kBlack, black_pawn->color());
  ASSERT_EQ(apollo::kPawn, black_pawn->kind());
  ASSERT_EQ(Square::E7, *p.EnPassantSquare());
}

TEST(PositionTest, MovingInvalidatesCastle) {
  Position p("8/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  ASSERT_TRUE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_TRUE(p.CanCastleQueenside(apollo::kWhite));

  p.MakeMove(Move::Quiet(Square::A1, Square::A2));
  ASSERT_TRUE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_FALSE(p.CanCastleQueenside(apollo::kWhite));
  p.UnmakeMove();
  ASSERT_TRUE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_TRUE(p.CanCastleQueenside(apollo::kWhite));

  p.MakeMove(Move::Quiet(Square::H1, Square::H2));
  ASSERT_FALSE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_TRUE(p.CanCastleQueenside(apollo::kWhite));
  p.UnmakeMove();
  ASSERT_TRUE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_TRUE(p.CanCastleQueenside(apollo::kWhite));

  p.MakeMove(Move::Quiet(Square::E1, Square::E2));
  ASSERT_FALSE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_FALSE(p.CanCastleQueenside(apollo::kWhite));
  p.UnmakeMove();
  ASSERT_TRUE(p.CanCastleKingside(apollo::kWhite));
  ASSERT_TRUE(p.CanCastleQueenside(apollo::kWhite));
}

TEST(PositionTest, BasicPromotion) {
  Position p("8/4P3/8/8/8/8/8/8 w - - 0 1");
  p.MakeMove(Move::Promotion(Square::E7, Square::E8, apollo::kQueen));
  auto white_queen = p.PieceAt(Square::E8);
  ASSERT_TRUE(white_queen.has_value());
  ASSERT_EQ(apollo::kWhite, white_queen->color());
  ASSERT_EQ(apollo::kQueen, white_queen->kind());
  p.UnmakeMove();
  auto white_pawn = p.PieceAt(Square::E7);
  ASSERT_TRUE(white_pawn.has_value());
  ASSERT_EQ(apollo::kWhite, white_pawn->color());
  ASSERT_EQ(apollo::kPawn, white_pawn->kind());
}

TEST(PositionTest, FenParseHalfmove) {
  Position p("8/8/8/8/8/4P3/8/8 w - - 5 6");
  ASSERT_EQ(5, p.HalfmoveClock());
  ASSERT_EQ(6, p.FullmoveClock());
}

TEST(PositionCheckTest, CheckSmoke) {
  Position p("8/8/3r4/8/8/8/8/3K4 w - -");
  ASSERT_TRUE(p.IsCheck(apollo::kWhite));
}

TEST(PositionCheckTest, CheckSmokeNeg) {
  Position p("8/8/4r3/8/8/8/8/3K4 w - -");
  ASSERT_FALSE(p.IsCheck(apollo::kWhite));
}

TEST(PositionPinTest, BasicPin) {
  Position p("8/8/4q3/8/8/8/4BB2/4K3 w - - 0 1");
  ASSERT_TRUE(p.IsAbsolutelyPinned(apollo::kBlack, Square::E2));
  ASSERT_FALSE(p.IsAbsolutelyPinned(apollo::kBlack, Square::F2));
}

TEST(PositionPinTest, BasicPinNeg) {
  Position p("8/8/4q3/8/4B3/8/4B3/4K3 w - - 0 1");
  ASSERT_FALSE(p.IsAbsolutelyPinned(apollo::kBlack, Square::E2));
}
