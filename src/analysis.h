#pragma once

#include "bitboard.h"
#include "position.h"
#include "types.h"

namespace apollo {

/**
 * The Analysis class provides common board analyses upon a static position.
 * It is suitable for use in board evaluators, where analysis queries can
 * be aggressively cached when evaluating a single, immutable board position.
 */
class Analysis {
 public:
  /**
   * Constructs an Analysis object for a given board position.
   */
  explicit Analysis(const Position& pos) : pos_(pos) {}

  /**
   * Returns the set of doubled pawns left by the given color.
   */
  Bitboard DoubledPawns(Color color);

  /**
   * Returns the set of backwards pawns left by the given color.
   */
  Bitboard BackwardPawns(Color color);

  /**
   * Returns the set of isolated pawns left by the given color.
   */
  Bitboard IsolatedPawns(Color color);

  /**
   * Returns the mobility of the given color, i.e. the number of legal moves
   * available.
   */
  int Mobility(Color color);

 private:
  Bitboard AdjacentFiles(File file);

  const Position& pos_;
};

}  // namespace apollo