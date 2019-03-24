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

TEST(MoveGenTest, PawnSmoke) {
  Position p("8/8/8/8/4P3/8/8/8 w - -");
  ASSERT_NO_FATAL_FAILURE(
      AssertHasMove(p, Move::Quiet(Square::E4, Square::E5)));
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