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
  BoardScore score;
};

class Searcher {
 public:
  explicit Searcher(std::unique_ptr<BoardEvaluator> eval)
      : evaluator_(std::move(eval)) {}

  SearchResult Search(Position& pos, int depth,
                      std::chrono::seconds time_budget);

 private:
  std::unique_ptr<BoardEvaluator> evaluator_;
};

}  // namespace apollo::search
