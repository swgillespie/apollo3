#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>

#include "types.h"
#include "util.h"

namespace apollo {

class Move {
  friend struct std::hash<Move>;

 public:
  static Move Quiet(Square src, Square dst) { return Move(src, dst); }

  static Move Capture(Square src, Square dst) {
    Move mov(src, dst);
    mov.bitset_.capture_bit_ = 1;
    return mov;
  }

  static Move EnPassant(Square src, Square dst) {
    Move mov = Capture(src, dst);
    mov.bitset_.special_1_bit_ = 1;
    return mov;
  }

  static Move DoublePawnPush(Square src, Square dst) {
    Move mov = Quiet(src, dst);
    mov.bitset_.special_1_bit_ = 1;
    return mov;
  }

  static Move Promotion(Square src, Square dst, PieceKind kind) {
    Move mov = Quiet(src, dst);
    mov.bitset_.promotion_bit_ = 1;
    switch (kind) {
      case kBishop:
        mov.bitset_.special_1_bit_ = 1;
        break;
      case kRook:
        mov.bitset_.special_0_bit_ = 1;
        break;
      case kQueen:
        mov.bitset_.special_0_bit_ = 1;
        mov.bitset_.special_1_bit_ = 1;
        break;
      default:
        break;
    }
    return mov;
  }

  static Move PromotionCapture(Square src, Square dst, PieceKind kind) {
    Move mov = Promotion(src, dst, kind);
    mov.bitset_.capture_bit_ = 1;
    return mov;
  }

  static Move KingsideCastle(Square src, Square dst) {
    Move mov = Quiet(src, dst);
    mov.bitset_.special_0_bit_ = 1;
    return mov;
  }

  static Move QueensideCastle(Square src, Square dst) {
    Move mov = Quiet(src, dst);
    mov.bitset_.special_0_bit_ = 1;
    mov.bitset_.special_1_bit_ = 1;
    return mov;
  }

  static Move Null() { return Quiet(Square::A1, Square::A1); }

  Square Source() const { return static_cast<Square>(bitset_.source_); }

  Square Destination() const { return static_cast<Square>(bitset_.dest_); }

  PieceKind PromotionPiece() const {
    assert(bitset_.promotion_bit_ &&
           "PromotionPiece only valid on promotion moves");
    if (bitset_.special_0_bit_ && bitset_.special_1_bit_) {
      return kQueen;
    }
    if (bitset_.special_0_bit_) {
      return kRook;
    }
    if (bitset_.special_1_bit_) {
      return kBishop;
    }
    return kKnight;
  }

  bool IsQuiet() const {
    return bitset_.promotion_bit_ == 0 && bitset_.capture_bit_ == 0 &&
           bitset_.special_0_bit_ == 0 && bitset_.special_1_bit_ == 0;
  }

  bool IsCapture() const { return bitset_.capture_bit_; }

  bool IsNull() const { return bits_ == 0; }

  bool IsKingsideCastle() const {
    return !bitset_.promotion_bit_ && !bitset_.capture_bit_ &&
           bitset_.special_0_bit_ && !bitset_.special_1_bit_;
  }

  bool IsQueensideCastle() const {
    return !bitset_.promotion_bit_ && !bitset_.capture_bit_ &&
           bitset_.special_0_bit_ && bitset_.special_1_bit_;
  }

  bool IsCastle() const { return IsKingsideCastle() || IsQueensideCastle(); }

  bool IsPromotion() const { return bitset_.promotion_bit_; }

  bool IsDoublePawnPush() const {
    return !bitset_.promotion_bit_ && !bitset_.capture_bit_ &&
           !bitset_.special_0_bit_ && bitset_.special_1_bit_;
  }

  std::string AsUci() const {
    std::stringstream str;
    str << util::SquareString(Source());
    str << util::SquareString(Destination());
    if (IsPromotion()) {
      switch (PromotionPiece()) {
        case kBishop:
          str << "b";
          break;
        case kKnight:
          str << "n";
          break;
        case kRook:
          str << "r";
          break;
        case kQueen:
          str << "q";
          break;
        default:
          break;
      }
    }
    return str.str();
  }

  bool operator==(const Move other) const { return other.bits_ == bits_; }

 private:
  Move(Square src, Square dst) {
    this->bitset_.promotion_bit_ = 0;
    this->bitset_.capture_bit_ = 0;
    this->bitset_.source_ = static_cast<uint8_t>(src);
    this->bitset_.special_0_bit_ = 0;
    this->bitset_.special_1_bit_ = 0;
    this->bitset_.dest_ = static_cast<uint8_t>(dst);
  }

  struct Bitset {
    bool promotion_bit_ : 1;
    bool capture_bit_ : 1;
    uint8_t source_ : 6;
    bool special_0_bit_ : 1;
    bool special_1_bit_ : 1;
    uint8_t dest_ : 6;
  };

  static_assert(sizeof(Bitset) == sizeof(uint16_t));
  union {
    Bitset bitset_;
    uint16_t bits_;
  };
};

static_assert(sizeof(Move) == sizeof(uint16_t));

}  // namespace apollo

namespace std {

template <>
struct hash<apollo::Move> {
  size_t operator()(const apollo::Move& mov) const {
    return hash<int>{}(mov.bits_);
  }
};

}  // namespace std
