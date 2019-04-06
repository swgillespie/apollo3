#pragma once

#include "position.h"

namespace apollo {

class BoardEvaluator {
 public:
  BoardEvaluator() {}

  virtual ~BoardEvaluator() {}

  virtual double Evaluate(const Position& pos) const = 0;
};

}  // namespace apollo
