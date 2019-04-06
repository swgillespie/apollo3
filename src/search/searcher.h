#pragma once

#include <chrono>
#include <memory>
#include <utility>

#include "board_evaluator.h"
#include "move.h"
#include "position.h"

namespace apollo::search {

struct SearchResult {
  Move best_move;
  double score;
  int nodes_searched;
};

class Searcher {
 public:
  explicit Searcher(std::unique_ptr<BoardEvaluator> eval)
      : evaluator_(std::move(eval)), nodes_(0) {}

  SearchResult Search(Position& pos, int depth);

 private:
  double AlphaBeta(Position& pos, double alpha, double beta, int depth);
  double Quiesce(Position& pos, double alpha, double beta);

  std::unique_ptr<BoardEvaluator> evaluator_;
  int nodes_;
};

}  // namespace apollo::search
