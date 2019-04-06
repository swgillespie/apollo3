#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include "evaluators/shannon_evaluator.h"
#include "position.h"
#include "search/searcher.h"

using apollo::Position;
using apollo::evaluators::ShannonEvaluator;
using apollo::search::Searcher;
using apollo::search::SearchResult;

namespace {

int depth = 2;
const char* position_fen = nullptr;

void ParseOptions(int argc, const char* argv[]) {
  int i = 2;
  while (i < argc) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      std::cout << "help!" << std::endl;
      std::exit(EXIT_FAILURE);
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

[[noreturn]] void EvaluateCommand(int argc, const char* argv[]) {
  ParseOptions(argc, argv);
  if (!position_fen) {
    std::cout << "no FEN position provided" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  Position p(position_fen);
  p.Dump(std::cout);

  Searcher search(std::make_unique<ShannonEvaluator>());
  SearchResult result = search.Search(p, depth);
  std::cout << "  best move: " << result.best_move << std::endl;
  std::cout << "      score: " << result.score << std::endl;
  std::cout << "      nodes: " << result.nodes_searched << std::endl;

  std::exit(EXIT_SUCCESS);
}