#pragma once

#include <cstdint>
#include <optional>

#include "attacks.h"
#include "types.h"

namespace apollo {

class Piece {
 public:
  static std::optional<Piece> FromChar(char c) {
    switch (c) {
      case 'p':
        return Piece(kBlack, kPawn);
      case 'n':
        return Piece(kBlack, kKnight);
      case 'b':
        return Piece(kBlack, kBishop);
      case 'r':
        return Piece(kBlack, kRook);
      case 'q':
        return Piece(kBlack, kQueen);
      case 'k':
        return Piece(kBlack, kKing);
      case 'P':
        return Piece(kWhite, kPawn);
      case 'N':
        return Piece(kWhite, kKnight);
      case 'B':
        return Piece(kWhite, kBishop);
      case 'R':
        return Piece(kWhite, kRook);
      case 'Q':
        return Piece(kWhite, kQueen);
      case 'K':
        return Piece(kWhite, kKing);
      default:
        return {};
    }
  }

  Piece(Color color, PieceKind kind) {
    this->color_ = color == kWhite ? 1 : 0;
    this->piece_ = static_cast<int>(kind);
  }

  Bitboard Attacks(Square sq, Bitboard occupancy) const {
    switch (kind()) {
      case kPawn:
        return attacks::PawnAttacks(sq, color());
      case kBishop:
        return attacks::BishopAttacks(sq, occupancy);
      case kKnight:
        return attacks::KnightAttacks(sq);
      case kRook:
        return attacks::RookAttacks(sq, occupancy);
      case kQueen:
        return attacks::QueenAttacks(sq, occupancy);
      case kKing:
        return attacks::KingAttacks(sq);
      default:
        CHECK(false) << "unknown piece kind";
        return Bitboard();
    }
  }

  Color color() const { return this->color_ == 1 ? kWhite : kBlack; }
  PieceKind kind() const { return static_cast<PieceKind>(this->piece_); }

  char AsChar() const {
    if (color() == kWhite) {
      switch (kind()) {
        case kPawn:
          return 'P';
        case kKnight:
          return 'N';
        case kBishop:
          return 'B';
        case kRook:
          return 'R';
        case kQueen:
          return 'Q';
        case kKing:
          return 'K';
        default:
          return '-';
      }
    } else {
      switch (kind()) {
        case kPawn:
          return 'p';
        case kKnight:
          return 'n';
        case kBishop:
          return 'b';
        case kRook:
          return 'r';
        case kQueen:
          return 'q';
        case kKing:
          return 'k';
        default:
          return '-';
      }
    }
  }

 private:
  uint16_t color_;
  uint16_t piece_;
};

}  // namespace apollo
