#pragma once

#include <cassert>
#include <cstdint>

#include "types.h"

namespace apollo {

class Move {
 public:
  static Move Quiet(Square src, Square dst) { return Move(src, dst); }

  static Move Capture(Square src, Square dst) {
    Move mov(src, dst);
    mov.capture_bit_ = 1;
    return mov;
  }

  static Move EnPassant(Square src, Square dst) {
    Move mov = Capture(src, dst);
    mov.special_1_bit_ = 1;
    return mov;
  }

  static Move DoublePawnPush(Square src, Square dst) {
    Move mov = Quiet(src, dst);
    mov.special_1_bit_ = 1;
    return mov;
  }

  static Move Promotion(Square src, Square dst, PieceKind kind) {
    Move mov = Quiet(src, dst);
    mov.promotion_bit_ = 1;
    switch (kind) {
      case kBishop:
        mov.special_1_bit_ = 1;
        break;
      case kRook:
        mov.special_0_bit_ = 1;
        break;
      case kQueen:
        mov.special_0_bit_ = 1;
        mov.special_1_bit_ = 1;
        break;
      default:
        break;
    }
    return mov;
  }

  static Move PromotionCapture(Square src, Square dst, PieceKind kind) {
    Move mov = Promotion(src, dst, kind);
    mov.capture_bit_ = 1;
    return mov;
  }

  static Move KingsideCastle(Square src, Square dst) {
    Move mov = Quiet(src, dst);
    mov.special_0_bit_ = 1;
    return mov;
  }

  static Move QueensideCastle(Square src, Square dst) {
    Move mov = Quiet(src, dst);
    mov.special_0_bit_ = 1;
    mov.special_1_bit_ = 1;
    return mov;
  }

  static Move Null() { return Quiet(Square::A1, Square::A1); }

  Square Source() const { return static_cast<Square>(this->source_); }

  Square Destination() const { return static_cast<Square>(this->dest_); }

  PieceKind PromotionPiece() const {
    assert(this->promotion_bit_ &&
           "PromotionPiece only valid on promotion moves");
    if (this->special_0_bit_ && this->special_1_bit_) {
      return kQueen;
    }
    if (this->special_0_bit_) {
      return kRook;
    }
    if (this->special_1_bit_) {
      return kBishop;
    }
    return kKnight;
  }

  bool IsQuiet() const {
    return this->promotion_bit_ == 0 && this->capture_bit_ == 0 &&
           this->special_0_bit_ == 0 && this->special_1_bit_ == 0;
  }

  bool IsCapture() const { return this->capture_bit_; }

  bool IsNull() const { return Source() == A1 && Destination() == A1; }

  bool IsKingsideCastle() const {
    return !promotion_bit_ && !capture_bit_ && special_0_bit_ &&
           !special_1_bit_;
  }

  bool IsQueensideCastle() const {
    return !promotion_bit_ && !capture_bit_ && special_0_bit_ && special_1_bit_;
  }

  bool IsCastle() const { return IsKingsideCastle() || IsQueensideCastle(); }

  bool IsPromotion() const { return promotion_bit_; }

  bool IsDoublePawnPush() const {
    return !promotion_bit_ && !capture_bit_ && !special_0_bit_ &&
           special_1_bit_;
  }

  bool operator==(const Move& other) {
    return other.promotion_bit_ == promotion_bit_ &&
           other.capture_bit_ == capture_bit_ && other.source_ == source_ &&
           other.special_0_bit_ && special_0_bit_ && other.special_1_bit_ &&
           special_1_bit_;
  }

 private:
  Move(Square src, Square dst) {
    this->promotion_bit_ = 0;
    this->capture_bit_ = 0;
    this->source_ = static_cast<uint8_t>(src);
    this->special_0_bit_ = 0;
    this->special_1_bit_ = 0;
    this->dest_ = static_cast<uint8_t>(dst);
  }

  bool promotion_bit_ : 1;
  bool capture_bit_ : 1;
  uint8_t source_ : 6;
  bool special_0_bit_ : 1;
  bool special_1_bit_ : 1;
  uint8_t dest_ : 6;
};

static_assert(sizeof(Move) == sizeof(uint16_t));

}  // namespace apollo
