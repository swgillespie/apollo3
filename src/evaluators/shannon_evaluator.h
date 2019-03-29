#include "../board_evaluator.h"
#include "../position.h"

namespace apollo {

/**
 * The ShannonEvaluator is a simple board evaluator based on the 1949 paper
 * "Programming a Computer for Playing Chess", where author Claude Shannon
 * proposes a simple board evaluation function to serve at the core of a
 * computer chess program.
 *
 * The ShannonEvaluator itself is derived from section 3 of the paper, where
 * Shannon provides the formula for a board evaluation function. It is simple,
 * yet powerful.
 *
 * See https://www.pi.infn.it/~carosi/chess/shannon.txt for the full text.
 */
class ShannonEvaluator : public BoardEvaluator {
  ShannonEvaluator();

  virtual BoardScore Evaluate(const Position& pos) override;
};

}  // namespace apollo
