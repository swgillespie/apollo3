#include <cctype>
#include <optional>
#include <sstream>
#include <string>

#include "attacks.h"
#include "movegen.h"
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
        if (Peek() == ' ') {
          break;
        }

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
  CHECK(!this->PieceAt(sq).has_value()) << "square is occupied already";
  this->boards_by_color_[piece.color()].Set(sq);
  size_t offset = piece.color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(piece.kind());
  this->boards_by_piece_[kind + offset].Set(sq);
}

void Position::RemovePiece(Square sq) {
  auto existing_piece = this->PieceAt(sq);
  CHECK(existing_piece.has_value()) << "square wasn't occupied";
  this->boards_by_color_[existing_piece->color()].Unset(sq);
  size_t offset = existing_piece->color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(existing_piece->kind());
  this->boards_by_piece_[kind + offset].Unset(sq);
}

std::optional<Piece> Position::PieceAt(Square sq) const {
  size_t board_offset;
  Color color;
  if (this->boards_by_color_[0].Test(sq)) {
    board_offset = 0;
    color = kWhite;
  } else if (this->boards_by_color_[1].Test(sq)) {
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

  CHECK(false) << "PieceAt fallthrough, bitboards are invalid";
  __builtin_unreachable();
}

Bitboard Position::SquaresAttacking(Color to_move, Square target) const {
  Bitboard occupancy = Pieces(kWhite) | Pieces(kBlack);
  Bitboard attacks;
  Queens(to_move).ForEach([&](Square queen) {
    if (attacks::QueenAttacks(queen, occupancy).Test(target)) {
      attacks.Set(queen);
    }
  });
  Rooks(to_move).ForEach([&](Square rook) {
    if (attacks::RookAttacks(rook, occupancy).Test(target)) {
      attacks.Set(rook);
    }
  });
  Bishops(to_move).ForEach([&](Square bishop) {
    if (attacks::BishopAttacks(bishop, occupancy).Test(target)) {
      attacks.Set(bishop);
    }
  });
  Knights(to_move).ForEach([&](Square knight) {
    if (attacks::KnightAttacks(knight).Test(target)) {
      attacks.Set(knight);
    }
  });
  Pawns(to_move).ForEach([&](Square pawn) {
    if (attacks::PawnAttacks(pawn, to_move).Test(target)) {
      attacks.Set(pawn);
    }
  });
  Kings(to_move).ForEach([&](Square king) {
    if (attacks::KingAttacks(king).Test(target)) {
      attacks.Set(king);
    }
  });
  if (current_state_.en_passant_square &&
      target == *current_state_.en_passant_square) {
    // TODO(sean)
  }
  return attacks;
}

bool Position::IsCheck(Color to_move) const {
  bool check = false;
  Kings(to_move).ForEach([&](Square king) {
    if (!SquaresAttacking(!to_move, king).Empty()) {
      check = true;
    }
  });
  return check;
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
  CHECK(moving_piece.has_value()) << "no piece at move source square";
  CHECK(!mov.IsPromotion()) << "NYI: Promotion";

  if (mov.IsCapture()) {
    Square target_square = mov.Destination();
    if (mov.IsEnPassant()) {
      // En-passant moves are the only case when the piece being captured does
      // not lie on the same square as the move destination.
      Direction ep_dir =
          side_to_move_ == kWhite ? kDirectionSouth : kDirectionNorth;
      CHECK(current_state_.en_passant_square) << "EP-move without EP-square";
      target_square = util::Towards(*current_state_.en_passant_square, ep_dir);
    }

    auto captured_piece = PieceAt(target_square);
    CHECK(captured_piece.has_value()) << "no piece at capture square";

    // Record the captured piece in the previous move's entry. When unwinding
    // the move stack (unmaking a move), we'll look at the previous move's entry
    // to determine what piece was captured.
    //
    // Note that this requires there to be at least one irreverisble state
    // already on the stack, but that's guaranteed because it's impossible for
    // the first move of a game of chess to be a capture.
    irreversible_state_.top().last_capture_ = captured_piece->kind();
    RemovePiece(target_square);
  }

  // Reset the EP-square. Either we took the EP-move and have already handled
  // it, or we didn't and it's no longer valid.
  current_state_.en_passant_square = {};
  if (mov.IsCastle()) {
    // Castles are encoded based on the king's start and stop position.
    // Notably, the rook is not at the move's destination square.
    Direction post_castle_dir, pre_castle_dir;
    int num_squares;
    if (mov.IsKingsideCastle()) {
      post_castle_dir = kDirectionWest;
      pre_castle_dir = kDirectionEast;
      num_squares = 1;
    } else {
      post_castle_dir = kDirectionEast;
      pre_castle_dir = kDirectionWest;
      num_squares = 2;
    }

    Square new_rook_square = util::Towards(mov.Destination(), post_castle_dir);
    Square rook_square =
        static_cast<Square>(static_cast<int>(mov.Destination()) +
                            kDirectionVectors[pre_castle_dir] * num_squares);

    auto rook = PieceAt(rook_square);
    CHECK(rook.has_value() && rook->kind() == kRook)
        << "rook not at destination";
    RemovePiece(rook_square);
    AddPiece(new_rook_square, *rook);
  }

  RemovePiece(mov.Source());
  AddPiece(mov.Destination(), *moving_piece);
  if (mov.IsDoublePawnPush()) {
    // Double-pawn pushes set the en passant square.
    Direction ep_dir =
        SideToMove() == kWhite ? kDirectionSouth : kDirectionNorth;
    Square ep_sq = util::Towards(mov.Destination(), ep_dir);
    current_state_.en_passant_square = ep_sq;
  }

  // Re-calculate our castling status. Side to move may have invalidated their
  // castle rights by moving their king or rooks.
  if (moving_piece->kind() == kRook) {
    // Moving a rook invalidates the castle on that rook's side of the board.
    Square kingside_rook, queenside_rook;
    if (side_to_move_ == kWhite) {
      kingside_rook = Square::H1;
      queenside_rook = Square::A1;
    } else {
      kingside_rook = Square::H8;
      queenside_rook = Square::A8;
    }

    if (CanCastleQueenside(side_to_move_) && mov.Source() == queenside_rook) {
      // Move of the queenside rook. Can't castle queenside anymore.
      TLOG() << "white can no longer castle queenside (rook move)";
      CastleStatus mask = side_to_move_ == kWhite ? kCastleWhiteQueenside
                                                  : kCastleBlackQueenside;
      current_state_.castle_status &= ~mask;
    }

    if (CanCastleKingside(side_to_move_) && mov.Source() == kingside_rook) {
      // Move of the kingside rook. Can't castle kingside anymore.
      TLOG() << "white can no longer castle kingside (rook move)";
      CastleStatus mask =
          side_to_move_ == kWhite ? kCastleWhiteKingside : kCastleBlackKingside;
      current_state_.castle_status &= ~mask;
    }
  } else if (moving_piece->kind() == kKing) {
    // Moving a king invalidates the castle on both sides.
    TLOG() << "white can no longer castle (king move)";
    CastleStatus mask = side_to_move_ == kWhite ? kCastleWhite : kCastleBlack;
    current_state_.castle_status &= ~mask;
  }

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
  CHECK(!irreversible_state_.empty()) << "no moves to unmake";
  current_state_ = irreversible_state_.top();
  irreversible_state_.pop();

  // Reverse the side to move.
  side_to_move_ = !side_to_move_;

  CHECK(current_state_.move.has_value()) << "no move available to unmake";
  Move mov = *current_state_.move;
  TLOG() << "Unmaking move: " << mov;
  CHECK(!mov.IsPromotion()) << "NYI: Promotion";

  // The rest of UnmakeMove proceeds in reverse of MakeMove; find the piece at
  // the destination square, remove it, replace it with the piece that was
  // captured, and move the piece back to the source.
  //
  // Since "the piece that was captured" is irreverisble state, we grab that
  // from current state (which we just restored).
  auto moved_piece = PieceAt(mov.Destination());
  CHECK(moved_piece.has_value()) << "no piece at move destination square";
  TLOG() << "piece that moved: " << moved_piece->kind();

  RemovePiece(mov.Destination());
  if (mov.IsCapture()) {
    CHECK(current_state_.last_capture_.has_value())
        << "unmaking capture with no last capture piece";
    TLOG() << "last captured piece was " << *current_state_.last_capture_;
    TLOG() << "side to move: " << side_to_move_;

    Square captured_piece_square = mov.Destination();
    if (mov.IsEnPassant()) {
      // Like in MakeMove, en passant is the only move where we have to put the
      // piece back somewhere other than the move destination.
      Direction ep_dir =
          side_to_move_ == kWhite ? kDirectionSouth : kDirectionNorth;
      captured_piece_square = util::Towards(mov.Destination(), ep_dir);
    }

    AddPiece(captured_piece_square,
             Piece(!side_to_move_, *current_state_.last_capture_));
  }
  AddPiece(mov.Source(), *moved_piece);

  if (mov.IsCastle()) {
    // If this move was a castle, we need to put the rook in the right spot.
    // If this is a kingside castle, the rook needs to be one square to the
    // east of the king. If this is a queenside castle, the rook needs to be
    // two squares to the west of the king.
    //
    // In both cases, mov.Destination() refers to the location of the king,
    // so we can calculate the rook position based on that.
    Square rook_square, target_rook_square;
    if (mov.IsKingsideCastle()) {
      rook_square = util::Towards(mov.Destination(), kDirectionWest);
      target_rook_square = util::Towards(mov.Destination(), kDirectionEast);
    } else {
      rook_square = util::Towards(mov.Destination(), kDirectionEast);
      target_rook_square = util::Towards(
          util::Towards(mov.Destination(), kDirectionWest), kDirectionWest);
    }

    auto rook = PieceAt(rook_square);
    CHECK(rook.has_value() && rook->kind() == kRook)
        << "rook not present at expected location";
    RemovePiece(rook_square);
    AddPiece(target_rook_square, *rook);
  }
}

std::vector<Move> Position::PseudolegalMoves() const {
  std::vector<Move> moves;
  moves.reserve(250);
  movegen::GeneratePseudolegalMoves(*this, moves);
  moves.shrink_to_fit();
  return moves;
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

std::string Position::AsFen() const {
  std::stringstream ss;
  for (int rank = kRank8; rank >= kRank1; rank--) {
    int empty_squares = 0;
    for (int file = kFileA; file < kFileLast; file++) {
      Square sq = util::SquareOf(rank, file);
      std::optional<Piece> piece_at_square = PieceAt(sq);
      if (piece_at_square) {
        if (empty_squares != 0) {
          ss << empty_squares;
        }
        ss << piece_at_square->AsChar();
        empty_squares = 0;
      } else {
        empty_squares++;
      }
    }

    if (empty_squares != 0) {
      ss << empty_squares;
    }

    if (rank != kRank1) {
      ss << "/";
    }
  }

  ss << " ";
  if (SideToMove() == kWhite) {
    ss << "w";
  } else {
    ss << "b";
  }
  ss << " ";
  if (CanCastleKingside(kWhite)) {
    ss << "K";
  }
  if (CanCastleQueenside(kWhite)) {
    ss << "Q";
  }
  if (CanCastleKingside(kBlack)) {
    ss << "k";
  }
  if (CanCastleQueenside(kBlack)) {
    ss << "q";
  }
  ss << " ";
  auto ep_square = EnPassantSquare();
  if (ep_square) {
    ss << util::SquareString(*ep_square);
  } else {
    ss << "-";
  }
  ss << " " << HalfmoveClock() << " " << FullmoveClock();
  return ss.str();
}

}  // namespace apollo
