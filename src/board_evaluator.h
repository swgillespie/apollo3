#pragma once

#include "position.h"

namespace apollo {

struct BoardScore {
  enum {
    kScore,
    kForseenWin,
    kForseenLoss,
  } tag;
  union {
    double score;
    int moves_to_mate;
  };

  bool operator==(const BoardScore& other) {
    if (tag == kScore && other.tag == kScore) {
      return score == other.score;
    }
    if (tag == other.tag) {
      return moves_to_mate == other.moves_to_mate;
    }
    return false;
  }

  bool operator!=(const BoardScore& other) { return !(*this == other); }
};

class BoardEvaluator {
 public:
  BoardEvaluator() {}

  virtual ~BoardEvaluator() {}

  virtual BoardScore Evaluate(const Position& pos) = 0;
};

}  // namespace apollo
