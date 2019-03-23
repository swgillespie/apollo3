#pragma once

#include "bitboard.h"
#include "types.h"

namespace apollo::attacks {

Bitboard PawnAttacks(Square sq, Color side);
Bitboard BishopAttacks(Square sq, Bitboard occupancy);
Bitboard RookAttacks(Square sq, Bitboard occupancy);
Bitboard QueenAttacks(Square sq, Bitboard occupancy);
Bitboard KnightAttacks(Square sq);
Bitboard KingAttacks(Square sq);

}  // namespace apollo::attacks
