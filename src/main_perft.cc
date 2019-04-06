#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "json.hpp"
#include "position.h"

using apollo::Color;
using apollo::Move;
using apollo::Position;
using nlohmann::json;

const char* const kIntermediateFilename = "intermediates.json";

namespace {

bool save_intermediates = false;
const char* position_fen = nullptr;
int depth = 4;

void ParseOptions(int argc, const char* argv[]) {
  int i = 2;
  while (i < argc) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      std::cout << "help!" << std::endl;
      std::exit(EXIT_FAILURE);
    }
    if (strcmp(argv[i], "--save-intermediates") == 0) {
      save_intermediates = true;
      i++;
      continue;
    }
    if (strcmp(argv[i], "--depth") == 0 || strcmp(argv[i], "-d") == 0) {
      i++;
      if (i >= argc) {
        std::cout << "expected argument for depth" << std::endl;
        std::exit(EXIT_FAILURE);
      }
      depth = atoi(argv[i++]);
      continue;
    }
    if (!position_fen) {
      position_fen = argv[i++];
    } else {
      std::cout << "unexpected positional argument: " << argv[i] << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
}

}  // anonymous namespace

int Perft(Position& pos, int depth, json& document) {
  if (depth == 0) {
    return 1;
  }

  json position;
  if (save_intermediates) {
    position["fen"] = pos.AsFen();
  }

  std::vector<std::string> moves;
  Color to_move = pos.SideToMove();
  int nodes = 0;
  for (Move mov : pos.PseudolegalMoves()) {
    pos.MakeMove(mov);
    if (!pos.IsCheck(to_move)) {
      if (save_intermediates) {
        moves.push_back(mov.AsUci());
      }
      nodes += Perft(pos, depth - 1, document);
    }
    pos.UnmakeMove();
  }

  if (save_intermediates) {
    position["moves"] = moves;
    document.push_back(position);
  }
  return nodes;
}

[[noreturn]] void PerftCommand(int argc, const char* argv[]) {
  ParseOptions(argc, argv);
  if (!position_fen) {
    std::cout << "no FEN position provided" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  if (save_intermediates) {
    std::cout << "saving intermediates" << std::endl;
  }

  json doc;
  Position p(position_fen);
  p.Dump(std::cout);
  std::cout << std::endl;
  for (int i = 1; i <= depth; i++) {
    int res = Perft(p, i, doc);
    std::cout << "perft(" << i << ") = " << res << std::endl;
  }

  if (save_intermediates) {
    std::ofstream fs;
    fs.open(kIntermediateFilename);
    fs << std::setw(4) << doc << std::endl;
    fs.close();
  }
  std::exit(EXIT_SUCCESS);
}
