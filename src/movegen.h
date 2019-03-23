#pragma once

#include <vector>

#include "move.h"
#include "position.h"

namespace apollo::movegen {

void GeneratePseudolegalMoves(const Position& pos, std::vector<Move>& moves);

}  // namespace apollo::movegen