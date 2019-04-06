#include <initializer_list>
#include <string_view>
#include <unordered_set>
#include "gtest/gtest.h"

#include "move.h"
#include "movegen.h"
#include "position.h"

using apollo::Move;
using apollo::Position;
using apollo::Square;

template <typename First>
void AssertHasMoveInSet(const std::unordered_set<Move>& move_set, First mov) {
  ASSERT_TRUE(move_set.find(mov) != move_set.end());
}

template <typename First, typename... Rest>
void AssertHasMoveInSet(const std::unordered_set<Move>& move_set, First mov,
                        Rest... rest) {
  ASSERT_TRUE(move_set.find(mov) != move_set.end());
  AssertHasMoveInSet(move_set, rest...);
}

template <typename First, typename... Rest>
void AssertHasMove(const Position& pos, First mov, Rest... rest) {
  std::vector<Move> moves = pos.PseudolegalMoves();
  std::unordered_set<Move> move_set(moves.begin(), moves.end());
  AssertHasMoveInSet(move_set, mov, rest...);
}

template <typename First>
void AssertDoesNotHaveMoveInSet(const std::unordered_set<Move>& move_set,
                                First mov) {
  ASSERT_TRUE(move_set.find(mov) == move_set.end());
}

template <typename First, typename... Rest>
void AssertDoesNotHaveMoveInSet(const std::unordered_set<Move>& move_set,
                                First mov, Rest... rest) {
  ASSERT_TRUE(move_set.find(mov) == move_set.end());
  AssertDoesNotHaveMoveInSet(move_set, rest...);
}

template <typename First, typename... Rest>
void AssertDoesNotHaveMove(const Position& pos, First mov, Rest... rest) {
  std::vector<Move> moves = pos.PseudolegalMoves();
  std::unordered_set<Move> move_set(moves.begin(), moves.end());
  AssertDoesNotHaveMoveInSet(move_set, mov, rest...);
}

TEST(MoveGenTest, PawnSmoke) {
  Position p("8/8/8/8/4P3/8/8/8 w - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::Quiet(Square::E4, Square::E5)));
}

TEST(MoveGenTest, PawnDoublePush) {
  Position p("8/8/8/8/8/8/4P3/8 w - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::DoublePawnPush(Square::E2, Square::E4)));
}

TEST(MoveGenTest, PawnCapture) {
  Position p("8/8/8/8/5p2/4P3/8/8 w - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::Capture(Square::E3, Square::F4)));
  ASSERT_NO_FATAL_FAILURE(
      AssertDoesNotHaveMove(p, Move::Capture(Square::E3, Square::D4)));
}

TEST(MoveGenTest, PawnSmokeBlack) {
  Position p("8/8/8/4p3/8/8/8/8 b - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::Quiet(Square::E5, Square::E4)));
}

TEST(MoveGenTest, PawnPromoSmoke) {
  Position p("8/4P3/8/8/8/8/8/8 w - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::Promotion(Square::E7, Square::E8, apollo::kKnight),
                    Move::Promotion(Square::E7, Square::E8, apollo::kBishop),
                    Move::Promotion(Square::E7, Square::E8, apollo::kRook),
                    Move::Promotion(Square::E7, Square::E8, apollo::kQueen)));
}

TEST(MoveGenTest, PawnPromoSmokeBlack) {
  Position p("8/8/8/8/8/8/4p3/8 b - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::Promotion(Square::E2, Square::E1, apollo::kKnight),
                    Move::Promotion(Square::E2, Square::E1, apollo::kBishop),
                    Move::Promotion(Square::E2, Square::E1, apollo::kRook),
                    Move::Promotion(Square::E2, Square::E1, apollo::kQueen)));
}

TEST(MoveGenTest, KnightSmoke) {
  Position p("8/8/4n3/8/3N4/8/8/8 w - -");
  ASSERT_NO_FATAL_FAILURE(AssertHasMove(
      p, Move::Quiet(Square::D4, Square::F5),
      Move::Quiet(Square::D4, Square::F3), Move::Quiet(Square::D4, Square::E2),
      Move::Quiet(Square::D4, Square::C2), Move::Quiet(Square::D4, Square::B3),
      Move::Quiet(Square::D4, Square::B5), Move::Quiet(Square::D4, Square::C6),
      Move::Capture(Square::D4, Square::E6)));
}

TEST(MoveGenTest, CastleWithNoRook) {
  // Bug fix - even if allowed to castle, White can't castle unless there's
  // actually a rook in its starting position.
  Position p("r3k2r/ppp2pp1/2n4p/3P4/8/bP4P1/P1PBBP1P/R3K2b w KQkq - 0 4");
  ASSERT_NO_FATAL_FAILURE(
      AssertDoesNotHaveMove(p, Move::KingsideCastle(Square::E1, Square::G1)));
}