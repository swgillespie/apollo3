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

void AssertMoveContains(std::string_view fen,
                        std::initializer_list<Move> moves) {
  Position p(fen);
  std::vector<Move> pseudolegal_moves = p.PseudolegalMoves();
  for (Move mov : moves) {
    // ASSERT_TRUE(pseudolegal_moves.find(mov) != pseudolegal_moves.end());
  }
}

TEST(MoveGenTest, Smoke) {}
