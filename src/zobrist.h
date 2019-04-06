#pragma once

#include <optional>

#include "piece.h"
#include "position.h"
#include "types.h"

namespace apollo::zobrist {

uint64_t Hash(const Position& pos);

void ModifyPiece(uint64_t& hash, Square square, Piece piece);
void ModifySideToMove(uint64_t& hash);
void ModifyKingsideCastle(uint64_t& hash, Color color);
void ModifyQueensideCastle(uint64_t& hash, Color color);
void ModifyEnPassant(uint64_t& hash, std::optional<Square> old_ep,
                     std::optional<Square> new_ep);

}  // namespace apollo::zobrist
