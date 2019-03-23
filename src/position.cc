#include <cctype>
#include <optional>
#include <sstream>
#include <string>

#include "attacks.h"
#include "piece.h"
#include "position.h"
#include "util.h"

namespace apollo {

class FenParser {
 public:
  FenParser(std::string_view fen) : it_(fen.cbegin()), end_(fen.cend()) {}

  void Parse(Position& pos) {
    for (int rank = kRank8; rank >= kRank1; rank--) {
      int file = kFileA;
      while (file <= kFileH) {
        char c = *it_;
        if (std::isdigit(c)) {
          if (c < '1' || c > '8') {
            throw InvalidFenException(kInvalidDigit);
          }

          file += static_cast<int>(c - 48);
          if (file > 8) {
            throw InvalidFenException(kFileInvalidSum);
          }

          Advance();
          continue;
        }

        auto piece = Piece::FromChar(c);
        if (!piece) {
          throw InvalidFenException(kUnexpectedChar);
        }

        Square sq = util::SquareOf(rank, file);
        pos.AddPiece(sq, *piece);
        Advance();
        file++;
      }

      if (rank != kRank1) {
        Eat('/');
      }
    }

    Eat(' ');

    // Side To Move
    switch (Peek()) {
      case 'w':
        pos.side_to_move_ = kWhite;
        break;
      case 'b':
        pos.side_to_move_ = kBlack;
        break;
      default:
        throw InvalidFenException(kUnexpectedChar);
    }
    Advance();
    Eat(' ');

    // Castle Status
    pos.current_state_.castle_status = kCastleNone;
    if (Peek() == '-') {
      Advance();
    } else {
      for (int i = 0; i < 4; i++) {
        switch (Peek()) {
          case 'K':
            pos.current_state_.castle_status |= kCastleWhiteKingside;
            break;
          case 'k':
            pos.current_state_.castle_status |= kCastleBlackKingside;
            break;
          case 'Q':
            pos.current_state_.castle_status |= kCastleWhiteQueenside;
            break;
          case 'q':
            pos.current_state_.castle_status |= kCastleBlackQueenside;
            break;
          default:
            throw InvalidFenException(kUnexpectedChar);
        }

        Advance();
      }
    }

    Eat(' ');

    // En Passant
    if (Peek() == '-') {
      Advance();
    } else {
      auto maybeFile = util::CharToFile(Peek());
      if (!maybeFile) {
        throw InvalidFenException(kUnexpectedChar);
      }
      Advance();
      auto maybeRank = util::CharToRank(Peek());
      if (!maybeRank) {
        throw InvalidFenException(kUnexpectedChar);
      }
      Advance();
      pos.current_state_.en_passant_square =
          util::SquareOf(*maybeRank, *maybeFile);
    }

    if (!PeekEof()) {
      return;
    }

    Eat(' ');
    // Halfmove Clock
    std::stringstream halfmove_stream;
    while (true) {
      char c = Peek();
      if (!std::isdigit(c)) {
        break;
      }
      halfmove_stream << c;
      Advance();
    }
    pos.current_state_.halfmove_clock = std::stoi(halfmove_stream.str());

    Eat(' ');
    // Fullmove Clock
    std::stringstream fullmove_stream;
    while (true) {
      auto c = PeekEof();
      if (!c || !std::isdigit(*c)) {
        break;
      }
      fullmove_stream << *c;
      Advance();
    }
    pos.current_state_.fullmove_clock = std::stoi(fullmove_stream.str());
  }

 private:
  void Eat(char c) {
    char next = Peek();
    if (next != c) {
      throw InvalidFenException(kUnexpectedChar);
    }
    Advance();
  }

  char Peek() {
    auto next = PeekEof();
    if (!next.has_value()) {
      throw InvalidFenException(kUnexpectedEOF);
    }
    return *next;
  }

  std::optional<char> PeekEof() {
    if (it_ == end_) {
      return {};
    }
    return *it_;
  }

  void Advance() { it_++; }

  std::string_view::const_iterator it_;
  std::string_view::const_iterator end_;
};

Position::Position(std::string_view fen)
    : current_state_(),
      irreversible_state_(),
      boards_by_piece_(),
      boards_by_color_(),
      side_to_move_(kWhite) {
  FenParser parser(fen);
  parser.Parse(*this);
}

void Position::AddPiece(Square sq, Piece piece) {
  assert(!this->PieceAt(sq).has_value() && "square is occupied already");
  this->boards_by_color_[piece.color()].Set(sq);
  size_t offset = piece.color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(piece.kind());
  this->boards_by_piece_[kind + offset].Set(sq);
}

void Position::RemovePiece(Square sq) {
  auto existing_piece = this->PieceAt(sq);
  assert(existing_piece.has_value() && "square wasn't occupied");
  this->boards_by_color_[existing_piece->color()].Unset(sq);
  size_t offset = existing_piece->color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(existing_piece->kind());
  this->boards_by_piece_[kind + offset].Unset(sq);
}

std::optional<Piece> Position::PieceAt(Square sq) const {
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

void Position::MakeMove(Move mov) {
  // MakeMove operates by maintaining a stack of the data that is irreversibly
  // lost whenever a move is made and maintaining a single copy of the data that
  // is reversibly lost.
  //
  // Before making a move, we must first copy the current irreversible state
  // onto the stack. Record the move that we're about to make, so we can replay
  // it backwards later.
  current_state_.move = mov;
  irreversible_state_.push(current_state_);

  if (mov.IsNull()) {
    // Quick out for null moves:
    //  1. EP is not legal next turn.

    //  2. Halfmove clock always increases.
    //  3. Fullmove clock increases if Black makes the null move.
    current_state_.en_passant_square = {};
    current_state_.halfmove_clock++;
    side_to_move_ = !side_to_move_;
    if (side_to_move_ == kWhite) {
      current_state_.fullmove_clock++;
    }
    return;
  }

  auto moving_piece = PieceAt(mov.Source());
  assert(moving_piece.has_value() && "no piece at move source square");
  assert(!mov.IsCapture() && "NYI: Capture");
  assert(!mov.IsCastle() && "NYI: Castle");
  assert(!mov.IsPromotion() && "NYI: Promotion");
  assert(!mov.IsDoublePawnPush() && "NYI: Double Pawn");

  RemovePiece(mov.Source());
  AddPiece(mov.Destination(), *moving_piece);
  side_to_move_ = !side_to_move_;
  if (mov.IsCapture() || moving_piece->kind() == kPawn) {
    current_state_.halfmove_clock = 0;
  } else {
    current_state_.halfmove_clock++;
  }

  if (side_to_move_ == kWhite) {
    current_state_.fullmove_clock++;
  }
}

void Position::UnmakeMove() {
  // To unmake the move, we must first restore the previous move's irreversible
  // state and then undo the reversible aspects of the move.
  assert(!irreversible_state_.empty() && "no moves to unmake");
  current_state_ = irreversible_state_.top();
  irreversible_state_.pop();

  assert(current_state_.move.has_value() && "no move available to unmake");
  Move mov = *current_state_.move;
  assert(!mov.IsCapture() && "NYI: Capture");
  assert(!mov.IsCastle() && "NYI: Castle");
  assert(!mov.IsPromotion() && "NYI: Promotion");
  assert(!mov.IsDoublePawnPush() && "NYI: Double Pawn");

  // The rest of UnmakeMove proceeds in reverse of MakeMove; find the piece at
  // the destination square, remove it, replace it with the piece that was
  // captured, and move the piece back to the source.
  //
  // Since "the piece that was captured" is irreverisble state, we grab that
  // from current state (which we just restored).
  auto moved_piece = PieceAt(mov.Destination());
  assert(moved_piece.has_value() && "no piece at move destination square");

  RemovePiece(mov.Destination());
  AddPiece(mov.Source(), *moved_piece);
}

void Position::Dump(std::ostream& out) const {
  for (int rank = kRank8; rank >= kRank1; rank--) {
    for (int file = kFileA; file < kFileLast; file++) {
      Square sq = util::SquareOf(rank, file);
      auto piece = PieceAt(sq);
      if (piece) {
        out << " " << piece->AsChar() << " ";
      } else {
        out << " . ";
      }
    }
    out << "| " << static_cast<char>(rank + 49) << std::endl;
  }
  for (int i = kFileA; i < kFileLast; i++) {
    out << "---";
  }
  out << std::endl;
  for (int i = kFileA; i < kFileLast; i++) {
    out << " " << static_cast<char>(i + 97) << " ";
  }
  out << std::endl;
}

}  // namespace apollo
