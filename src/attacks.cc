#include <array>

#include "attacks.h"
#include "log.h"

namespace apollo {

namespace {

class KingTable {
 public:
  constexpr KingTable() {
    for (int i = A1; i < kSquareLast; i++) {
      Square sq = static_cast<Square>(i);
      Bitboard board;
      if (!kBBRank8.Test(sq)) {
        board.Set(static_cast<Square>(i + 8));
        if (!kBBFileA.Test(sq)) {
          board.Set(static_cast<Square>(i + 7));
        }
        if (!kBBFileH.Test(sq)) {
          board.Set(static_cast<Square>(i + 9));
        }
      }

      if (!kBBRank1.Test(sq)) {
        board.Set(static_cast<Square>(i - 8));
        if (!kBBFileA.Test(sq)) {
          board.Set(static_cast<Square>(i - 9));
        }
        if (!kBBFileH.Test(sq)) {
          board.Set(static_cast<Square>(i - 7));
        }
      }

      if (!kBBFileA.Test(sq)) {
        board.Set(static_cast<Square>(i - 1));
      }
      if (!kBBFileH.Test(sq)) {
        board.Set(static_cast<Square>(i + 1));
      }
      table_[i] = board;
    }
  }

  Bitboard Attacks(Square sq) const { return table_[static_cast<size_t>(sq)]; }

 private:
  std::array<Bitboard, kSquareLast> table_;
};

class PawnTable {
 public:
  constexpr PawnTable() {
    for (int i = A1; i < kSquareLast; i++) {
      Square sq = static_cast<Square>(i);
      for (auto color : {kWhite, kBlack}) {
        Bitboard board;
        Bitboard promo_rank = color == kWhite ? kBBRank8 : kBBRank1;
        int up_left = color == kWhite ? 7 : -9;
        int up_right = color == kWhite ? 9 : -7;

        if (promo_rank.Test(sq)) {
          // No legal moves for this particular pawn. It's generally impossible
          // for pawns to be on the promotion rank anyway since they should have
          // been promoted already.
          continue;
        }

        if (!kBBFileA.Test(sq)) {
          board.Set(static_cast<Square>(i + up_left));
        }
        if (!kBBFileH.Test(sq)) {
          board.Set(static_cast<Square>(i + up_right));
        }
        table_[i][color] = board;
      }
    }
  }

  Bitboard Attacks(Square sq, Color side) const {
    return table_[static_cast<size_t>(sq)][side];
  }

 private:
  std::array<std::array<Bitboard, kColorLast>, kSquareLast> table_;
};

class RayTable {
 public:
  constexpr RayTable() {
    for (int i = A1; i < kSquareLast; i++) {
      Square sq = static_cast<Square>(i);

      auto populate_dir = [&](Direction dir, Bitboard edge) {
        Bitboard entry;
        if (edge.Test(sq)) {
          // Nothing to do here, there are no legal moves on this ray from this
          // square.
          this->table_[sq][dir] = entry;
          return;
        }

        // Starting at the given square, cast a ray in the given direction and
        // add all bits to the ray mask.
        int cursor = i;
        while (true) {
          cursor = cursor + kDirectionVectors[dir];
          Square cursorSq = static_cast<Square>(cursor);
          entry.Set(cursorSq);

          // Did we reach the end of the board? If so, stop.
          if (edge.Test(cursorSq)) {
            break;
          }
        }
        this->table_[sq][dir] = entry;
      };

      populate_dir(kDirectionNorth, kBBRank8);
      populate_dir(kDirectionNorthEast, kBBRank8 | kBBFileH);
      populate_dir(kDirectionEast, kBBFileH);
      populate_dir(kDirectionSouthEast, kBBRank1 | kBBFileH);
      populate_dir(kDirectionSouth, kBBRank1);
      populate_dir(kDirectionSouthWest, kBBRank1 | kBBFileA);
      populate_dir(kDirectionWest, kBBFileA);
      populate_dir(kDirectionNorthWest, kBBRank8 | kBBFileA);
    }
  }

  Bitboard Attacks(Square sq, Direction dir) const {
    return this->table_[static_cast<size_t>(sq)][static_cast<size_t>(dir)];
  }

 private:
  std::array<std::array<Bitboard, kDirectionLast>, kSquareLast + 1> table_;
};

class KnightTable {
 public:
  constexpr KnightTable() {
    for (int i = A1; i < kSquareLast; i++) {
      Square sq = static_cast<Square>(i);
      Bitboard board;
      if (!kBBFileA.Test(sq) && !kBBRank78.Test(sq)) {
        board.Set(static_cast<Square>(i + 15));
      }
      if (!kBBFileH.Test(sq) && !kBBRank78.Test(sq)) {
        board.Set(static_cast<Square>(i + 17));
      }
      if (!kBBFileGH.Test(sq) && !kBBRank8.Test(sq)) {
        board.Set(static_cast<Square>(i + 10));
      }
      if (!kBBFileGH.Test(sq) && !kBBRank1.Test(sq)) {
        board.Set(static_cast<Square>(i - 6));
      }
      if (!kBBFileH.Test(sq) && !kBBRank12.Test(sq)) {
        board.Set(static_cast<Square>(i - 15));
      }
      if (!kBBFileA.Test(sq) && !kBBRank12.Test(sq)) {
        board.Set(static_cast<Square>(i - 17));
      }
      if (!kBBFileAB.Test(sq) && !kBBRank1.Test(sq)) {
        board.Set(static_cast<Square>(i - 10));
      }
      if (!kBBFileAB.Test(sq) && !kBBRank8.Test(sq)) {
        board.Set(static_cast<Square>(i + 6));
      }
      this->table_[i] = board;
    }
  }

  constexpr Bitboard Attacks(Square sq) const {
    return this->table_[static_cast<size_t>(sq)];
  }

 private:
  std::array<Bitboard, kSquareLast> table_;
};

constexpr KingTable kKingTable = KingTable();
constexpr PawnTable kPawnTable = PawnTable();
constexpr RayTable kRayTable = RayTable();
constexpr KnightTable kKnightTable = KnightTable();

/**
 * Calculates the attacks of a positive ray starting at the given square, going
 * the given direction, and with the given board occupancy. The final square of
 * a ray is set as an attack even if the square is occupied, because it is
 * possible that the occupant there is an enemy piece and the move is a legal
 * attack move.
 *
 * A ray is positive if its direction vector is positive.
 */
Bitboard PositiveRayAttacks(Square sq, Bitboard occupancy, Direction dir) {
  CHECK(kDirectionVectors[dir] > 0) << "direction vector must be positive";
  Bitboard attacks = kRayTable.Attacks(sq, dir);
  uint64_t blocker = (attacks & occupancy).Bits();
  uint64_t blocking_square = blocker == 0 ? 64 : __builtin_ctzll(blocker);
  Bitboard blocking_ray =
      kRayTable.Attacks(static_cast<Square>(blocking_square), dir);
  return attacks ^ blocking_ray;
}

Bitboard NegativeRayAttacks(Square sq, Bitboard occupancy, Direction dir) {
  CHECK(kDirectionVectors[dir] < 0) << "direction vector must be negative";
  Bitboard attacks = kRayTable.Attacks(sq, dir);
  uint64_t blocker = (attacks & occupancy).Bits();
  uint64_t blocking_square;
  if (blocker == 0) {
    blocking_square = 64;
  } else {
    blocking_square = 63 - __builtin_clzll(blocker);
  }
  Bitboard blocking_ray =
      kRayTable.Attacks(static_cast<Square>(blocking_square), dir);
  return attacks ^ blocking_ray;
}

Bitboard DiagonalAttacks(Square sq, Bitboard occupancy) {
  return PositiveRayAttacks(sq, occupancy, kDirectionNorthWest) |
         NegativeRayAttacks(sq, occupancy, kDirectionSouthEast);
}

Bitboard AntidiagonalAttacks(Square sq, Bitboard occupancy) {
  return PositiveRayAttacks(sq, occupancy, kDirectionNorthEast) |
         NegativeRayAttacks(sq, occupancy, kDirectionSouthWest);
}

Bitboard FileAttacks(Square sq, Bitboard occupancy) {
  return PositiveRayAttacks(sq, occupancy, kDirectionNorth) |
         NegativeRayAttacks(sq, occupancy, kDirectionSouth);
}

Bitboard RankAttacks(Square sq, Bitboard occupancy) {
  return PositiveRayAttacks(sq, occupancy, kDirectionEast) |
         NegativeRayAttacks(sq, occupancy, kDirectionWest);
}

}  // anonymous namespace

namespace attacks {

Bitboard PawnAttacks(Square sq, Color side) {
  return kPawnTable.Attacks(sq, side);
}

Bitboard BishopAttacks(Square sq, Bitboard occupancy) {
  return DiagonalAttacks(sq, occupancy) | AntidiagonalAttacks(sq, occupancy);
}

Bitboard RookAttacks(Square sq, Bitboard occupancy) {
  return FileAttacks(sq, occupancy) | RankAttacks(sq, occupancy);
}

Bitboard QueenAttacks(Square sq, Bitboard occupancy) {
  return BishopAttacks(sq, occupancy) | RookAttacks(sq, occupancy);
}

Bitboard KnightAttacks(Square sq) { return kKnightTable.Attacks(sq); }

Bitboard KingAttacks(Square sq) { return kKingTable.Attacks(sq); }

}  // namespace attacks

}  // namespace apollo
