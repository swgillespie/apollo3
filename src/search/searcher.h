#pragma once

#include <chrono>
#include <memory>
#include <utility>

#include "board_evaluator.h"
#include "move.h"
#include "position.h"
#include "transposition_table.h"

namespace apollo::search {

struct SearchResult {
  Move best_move;
  double score;
};

class Searcher {
 public:
  explicit Searcher(std::unique_ptr<BoardEvaluator> eval)
      : evaluator_(std::move(eval)), transposition_table_(), search_state_() {}

  SearchResult Search(Position& pos, int depth,
                      std::chrono::seconds time_budget);

 private:
  double AlphaBeta(Position& pos, double alpha, double beta, int depth);
  double Quiesce(Position& pos, double alpha, double beta);

  using TimePoint = std::chrono::system_clock::time_point;

  struct PerSearchState {
    TimePoint search_start;
  };

  // Persistent state
  std::unique_ptr<BoardEvaluator> evaluator_;
  TranspositionTable transposition_table_;
  PerSearchState search_state_;

  // Per-search state
};

}  // namespace apollo::search
