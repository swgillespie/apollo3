#pragma once

#include <cstdint>

#include "types.h"

namespace apollo {

class Piece {
 public:
  Piece(Color color, PieceKind kind) {
    this->color_ = color == kWhite ? 1 : 0;
    this->piece_ = static_cast<int>(kind);
  }

  Color Color() const { return this->color_ == 1 ? kWhite : kBlack; }
  PieceKind Kind() const { return static_cast<PieceKind>(this->piece_); }

 private:
  uint16_t color_;
  uint16_t piece_;
};

}  // namespace apollo
