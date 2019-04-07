#include <cstring>
#include <fstream>
#include <iostream>

#include "attacks.h"
#include "bitboard.h"
#include "log.h"
#include "move.h"
#include "position.h"
#include "uci.h"

using apollo::Bitboard;
using apollo::Move;
using apollo::Position;
using apollo::Square;
using apollo::UciServer;

const char* kUsage =
    R"USG(Apollo3 chess engine, by Sean Gillespie <sean@swgillespie.me>
Usage:
  apollo3 perft <position>    # To analyze a position using PERFT
  apollo3 evaluate <position> # Evaluate the best move from a given position
  apollo3                     # To play a game of chess
)USG";

[[noreturn]] void PerftCommand(int argc, const char* argv[]);
[[noreturn]] void EvaluateCommand(int argc, const char* argv[]);

int main(int argc, const char* argv[]) {
  apollo::LogEnable(apollo::kLogInfo);
  TLOG() << "logging enabled";
  /*
  Position p("rnbqkbnr/pp2pppp/2p5/3P4/3P4/8/PPP2PPP/RNBQKBNR w KQkq -");
  p.Dump(std::cout);
  p.MakeMove(Move::Quiet(Square::F2, Square::F3));
  p.Dump(std::cout);
  */
  if (argc >= 2 && strcmp(argv[1], "perft") == 0) {
    PerftCommand(argc, argv);
  }
  if (argc >= 2 && strcmp(argv[1], "evaluate") == 0) {
    EvaluateCommand(argc, argv);
  }

  std::ofstream log("/var/log/apollo3.log");
  log << "----- beginning new session" << std::endl;
  UciServer server(std::cin, std::cout, log);
  server.Run();
  // std::cout << kUsage << std::endl;
}
