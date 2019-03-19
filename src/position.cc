#include "position.h"

namespace apollo {

namespace {

class FenParser {
 public:
  FenParser(std::string_view fen) : it_(fen.cbegin()), end_(fen.cend()) {}

  bool Parse(Position& pos) {}

 private:
  std::string_view::const_iterator it_;
  std::string_view::const_iterator end_;
};

}  // anonymous namespace

Position::Position(std::string_view fen)
    : current_state_(),
      irreversible_state_(),
      boards_by_piece_(),
      boards_by_color_(),
      side_to_move_(kWhite) {
  auto it = fen.cbegin();
  auto end = fen.cend();
}

void Position::AddPiece(Square sq, Piece piece) {
  assert(!this->PieceAt(sq).has_value() && "square is occupied already");
  this->boards_by_color_[piece.Color()].Set(sq);
  size_t offset = piece.Color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(piece.Kind());
  this->boards_by_piece_[kind + offset].Set(sq);
}

void Position::RemovePiece(Square sq) {
  auto existing_piece = this->PieceAt(sq);
  assert(existing_piece.has_value() && "square wasn't occupied");
  this->boards_by_color_[existing_piece->Color()].Unset(sq);
  size_t offset = existing_piece->Color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(existing_piece->Kind());
  this->boards_by_piece_[kind + offset].Unset(sq);
}

std::optional<Piece> Position::PieceAt(Square sq) {
  size_t board_offset;
  Color color;
  if (this->boards_by_color_[kWhite].Test(sq)) {
    board_offset = 0;
    color = kWhite;
  } else if (this->boards_by_color_[kBlack].Test(sq)) {
    board_offset = 6;
    color = kBlack;
  } else {
    return {};
  }

  for (size_t i = 0; i < 6; i++) {
    if (this->boards_by_piece_[i + board_offset].Test(sq)) {
      return Piece(color, static_cast<PieceKind>(i));
    }
  }

  assert(!"PieceAt fallthrough, bitboards are invalid");
  __builtin_unreachable();
}

void Position::MakeMove(Move mov) { assert("NYI"); }

void Position::UnmakeMove(Move mov) { assert("NYI"); }

};  // namespace apollo
