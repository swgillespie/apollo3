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

TEST(PerftTest, Start1) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 1, 20);
}

TEST(PerftTest, Start2) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 2, 400);
}

TEST(PerftTest, Start3) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 3, 8902);
}

TEST(PerftTest, Start4) {
  Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  ASSERT_PERFT(p, 4, 197281);
}

TEST(PerftTest, Kiwipete1) {
  Position p(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  ASSERT_PERFT(p, 1, 48);
}

TEST(PerftTest, Kiwipete2) {
  Position p(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  ASSERT_PERFT(p, 2, 2039);
}

TEST(PerftTest, Kiwipete3) {
  Position p(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  ASSERT_PERFT(p, 3, 97862);
}

TEST(PerftTest, Position3_1) {
  Position p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
  ASSERT_PERFT(p, 1, 14);
}

TEST(PerftTest, Position3_2) {
  Position p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
  ASSERT_PERFT(p, 2, 191);
}

TEST(PerftTest, Position3_3) {
  Position p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
  ASSERT_PERFT(p, 3, 2812);
}

TEST(PerftTest, Position3_4) {
  Position p("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
  ASSERT_PERFT(p, 4, 43238);
}
