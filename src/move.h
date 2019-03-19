#pragma once

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
      case PieceKind::Bishop:
        mov.special_1_bit_ = 1;
        break;
      case PieceKind::Rook:
        mov.special_0_bit_ = 1;
        break;
      case PieceKind::Queen:
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
      return PieceKind::Queen;
    }
    if (this->special_0_bit_) {
      return PieceKind::Rook;
    }
    if (this->special_1_bit_) {
      return PieceKind::Bishop;
    }
    return PieceKind::Knight;
  }

  bool IsQuiet() const {
    return this->promotion_bit_ == 0 && this->capture_bit_ == 0 &&
           this->special_0_bit_ == 0 && this->special_1_bit_ == 0;
  }

  bool IsCapture() const { return this->capture_bit_; }

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

};  // namespace apollo
