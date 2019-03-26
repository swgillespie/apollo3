#include "gtest/gtest.h"

#include "move.h"
#include "position.h"
#include "types.h"

using apollo::Color;
using apollo::Move;
using apollo::Position;

uint64_t Perft(Position& pos, int depth) {
  if (depth == 0) {
    return 1;
  }

  Color to_move = pos.SideToMove();
  int nodes = 0;
  for (Move mov : pos.PseudolegalMoves()) {
    pos.MakeMove(mov);
    if (!pos.IsCheck(to_move)) {
      nodes += Perft(pos, depth - 1);
    }
    pos.UnmakeMove();
  }
  return nodes;
}

#define ASSERT_PERFT(pos, depth, count) ASSERT_EQ(count, Perft(pos, depth))

TEST(PerfTests, Start1) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 1, 20);
}

TEST(PerfTests, Start2) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 2, 400);
}

TEST(DISABLED_PerfTests, Start3) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 3, 8902);
}