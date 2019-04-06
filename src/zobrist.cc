#include <array>
#include <cstdint>

#include "util.h"
#include "zobrist.h"

namespace apollo::zobrist {

class Xorshift64RandomGenerator {
 public:
  constexpr Xorshift64RandomGenerator(uint64_t seed) : state_(seed) {}

  constexpr uint64_t Next() {
    uint64_t x = state_;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    state_ = x;
    return state_;
  }

 private:
  uint64_t state_;
};

class ZobristHasher {
 public:
  const size_t kSideToMoveIndex = 768;
  const size_t kCastlingRightsIndex = 769;
  const size_t kEnPassantIndex = 773;

  constexpr ZobristHasher(uint64_t seed) : rng_(seed), magic_hashes_() {
    for (uint64_t& entry : magic_hashes_) {
      entry = rng_.Next();
    }
  }

  uint64_t SquareHash(PieceKind kind, Color color, Square square) const {
    // The layout of this table is:
    // [square]
    //   0 white pawn hash
    //   1 white knight hash
    //   ...
    //   5 white king hash
    //   6 black pawn hash
    //   7 black knight hash
    //   ...
    //   11 black king hash
    //
    // So, the square base is 12 * square, since the table is laid out one
    // square after another.
    size_t square_offset = 12 * static_cast<size_t>(square);
    size_t color_offset = color == kWhite ? 0 : 6;
    size_t piece_offset = static_cast<size_t>(kind);
    return magic_hashes_[square_offset + color_offset + piece_offset];
  }

  uint64_t SideToMoveHash(Color color) const {
    if (color == kWhite) {
      return 0;
    } else {
      return magic_hashes_[kSideToMoveIndex];
    }
  }

  uint64_t EnPassantHash(Square ep_square) const {
    File file = util::FileOf(ep_square);
    return magic_hashes_[static_cast<size_t>(file) + kEnPassantIndex];
  }

  uint64_t CastleHash(size_t offset) const {
    return magic_hashes_[offset + kCastlingRightsIndex];
  }

  uint64_t Hash(const Position& pos) const {
    uint64_t running_hash = 0;
    for (Square sq : kSquares) {
      for (Color color : kColors) {
        for (PieceKind piece : kPieces) {
          if (pos.Pieces(color, piece).Test(sq)) {
            running_hash ^= SquareHash(piece, color, sq);
          }
        }
      }
    }

    running_hash ^= SideToMoveHash(pos.SideToMove());
    if (pos.CanCastleKingside(kWhite)) {
      running_hash ^= CastleHash(0);
    }
    if (pos.CanCastleQueenside(kWhite)) {
      running_hash ^= CastleHash(1);
    }
    if (pos.CanCastleKingside(kBlack)) {
      running_hash ^= CastleHash(2);
    }
    if (pos.CanCastleKingside(kBlack)) {
      running_hash ^= CastleHash(3);
    }
    if (pos.EnPassantSquare()) {
      running_hash ^= EnPassantHash(*pos.EnPassantSquare());
    }
    return running_hash;
  }

 private:
  Xorshift64RandomGenerator rng_;
  std::array<uint64_t, 781> magic_hashes_;
};

const uint64_t kSeed = 0xf68e34a4e8ccf09a;
constexpr ZobristHasher kHasher = ZobristHasher(kSeed);

uint64_t Hash(const Position& pos) { return kHasher.Hash(pos); }

void ModifyPiece(uint64_t& hash, Square square, Piece piece) {
  hash ^= kHasher.SquareHash(piece.kind(), piece.color(), square);
}

void ModifySideToMove(uint64_t& hash) {
  hash ^= kHasher.SideToMoveHash(kBlack);
}

void ModifyKingsideCastle(uint64_t& hash, Color color) {
  size_t offset = color == kWhite ? 0 : 2;
  hash ^= kHasher.CastleHash(offset);
}

void ModifyQueensideCastle(uint64_t& hash, Color color) {
  size_t offset = color == kWhite ? 1 : 3;
  hash ^= kHasher.CastleHash(offset);
}

void ModifyEnPassant(uint64_t& hash, std::optional<Square> old_ep,
                     std::optional<Square> new_ep) {
  if (!old_ep && !new_ep) {
    return;
  }

  if (old_ep && !new_ep) {
    hash ^= kHasher.EnPassantHash(*old_ep);
    return;
  }

  if (!old_ep && new_ep) {
    hash ^= kHasher.EnPassantHash(*new_ep);
    return;
  }

  hash ^= kHasher.EnPassantHash(*old_ep);
  hash ^= kHasher.EnPassantHash(*new_ep);
}

}  // namespace apollo::zobrist
