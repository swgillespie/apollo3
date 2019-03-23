#include <iostream>

#include "attacks.h"
#include "bitboard.h"
#include "move.h"
#include "position.h"

using apollo::Bitboard;
using apollo::Move;
using apollo::Position;
using apollo::Square;

int main() {
  /*
  Position p("rnbqkbnr/pp2pppp/2p5/3P4/3P4/8/PPP2PPP/RNBQKBNR w KQkq -");
  p.Dump(std::cout);
  p.MakeMove(Move::Quiet(Square::F2, Square::F3));
  p.Dump(std::cout);
  */

  Bitboard occ;
  occ.Set(Square::C3);
  Bitboard b = apollo::attacks::QueenAttacks(Square::E5, occ);
  b.Dump(std::cout);
}
