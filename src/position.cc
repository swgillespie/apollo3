#include <cctype>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>

#include "attacks.h"
#include "movegen.h"
#include "piece.h"
#include "position.h"
#include "util.h"
#include "zobrist.h"

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
    pos.current_state_.zobrist_hash_ = zobrist::Hash(pos);
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
  zobrist::ModifyPiece(current_state_.zobrist_hash_, sq, piece);
}

void Position::RemovePiece(Square sq) {
  auto existing_piece = this->PieceAt(sq);
  CHECK(existing_piece.has_value()) << "square wasn't occupied";
  this->boards_by_color_[existing_piece->color()].Unset(sq);
  size_t offset = existing_piece->color() == kWhite ? 0 : 6;
  size_t kind = static_cast<size_t>(existing_piece->kind());
  this->boards_by_piece_[kind + offset].Unset(sq);
  zobrist::ModifyPiece(current_state_.zobrist_hash_, sq, *existing_piece);
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

bool Position::IsAbsolutelyPinned(Color to_move, Square sq) const {
  auto maybe_pinned_piece = PieceAt(sq);
  if (!maybe_pinned_piece) {
    // If there's no piece on the requested square, it's not pinned.
    return false;
  }

  if (maybe_pinned_piece->kind() == kKing) {
    // Kings can't be pinned.
    return false;
  }

  Bitboard attacks_on_square = SquaresAttacking(to_move, sq);
  if (attacks_on_square.Empty()) {
    // Nobody is attacking this square? Not a pin.
    return false;
  }

  Bitboard kings = Kings(!to_move);
  Bitboard all_pieces_except_pin = Pieces(kWhite) | Pieces(kBlack);
  all_pieces_except_pin.Unset(sq);
  for (auto it = attacks_on_square.Iterator(); it.HasNext();) {
    Square attacker = it.Next();
    Piece piece = PieceAt(attacker).value();
    switch (piece.kind()) {
      case kPawn:
      case kKnight:
      case kKing:
        // Pawns, knights, and kings can't participate in pins.
        continue;
      case kBishop:
        if (!(attacks::BishopAttacks(attacker, all_pieces_except_pin) & kings)
                 .Empty()) {
          TLOG() << "bishop on square " << util::SquareString(attacker)
                 << " absolutely pins " << sq;
          return true;
        }
        break;
      case kRook:
        if (!(attacks::RookAttacks(attacker, all_pieces_except_pin) & kings)
                 .Empty()) {
          TLOG() << "rook on square " << util::SquareString(attacker)
                 << " absolutely pins " << util::SquareString(sq);
          return true;
        }
        break;
      case kQueen:
        if (!(attacks::QueenAttacks(attacker, all_pieces_except_pin) & kings)
                 .Empty()) {
          TLOG() << "queen on square " << util::SquareString(attacker)
                 << " absolutely pins " << util::SquareString(sq);
          return true;
        }
        break;
      default:
        break;
    }
  }

  return false;
}

bool Position::IsLegal(Move mov) const {
  // This is a very naive implementation of pseudo-legality, based on the fact
  // that we know that the move generator generates pseudo legal moves.
  std::vector<Move> pseudolegal_moves = PseudolegalMoves();
  std::unordered_set<Move> move_set(pseudolegal_moves.begin(),
                                    pseudolegal_moves.end());
  if (move_set.find(mov) == move_set.end()) {
    // Moves that are not pseudolegal are also not legal.
    return false;
  }

  return IsLegalGivenPseudolegal(mov);
}

bool Position::IsLegalGivenPseudolegal(Move mov) const {
  auto moving_piece = PieceAt(mov.Source());
  if (!moving_piece) {
    return false;
  }

  // At this point we've confirmed that the move is pseudolegal. For a move to
  // be legal, it must not leave the king in check after the move. This has two
  // meanings, depending on the current state of the board:
  //
  //   1. If the player to move is in check, the given move must leave them out
  //   of check
  //   2. If the player to move is not in check, the given move must keep them
  //   out of check
  //
  // Therefore this function proceeds differently depending on whether or not
  // we're in check.
  Color to_move = SideToMove();
  if (IsCheck(to_move)) {
    CHECK(Kings(to_move).Count() == 1) << "expected exactly one king";
    Square king = Kings(to_move).Iterator().Next();
    Bitboard checking_pieces = SquaresAttacking(!to_move, king);
    if (checking_pieces.Count() > 1) {
      // Double or greater check. It is only legal to move a king.
      if (moving_piece->kind() != kKing) {
        return false;
      }

      // Fall-through to the remainder of this function. We're moving a king;
      // the rest of the function will validate that the king's not moving to a
      // checked square.
    } else {
      CHECK(checking_pieces.Count() == 1)
          << "should be exactly one checking piece";
      Square checking_piece_square = checking_pieces.Iterator().Next();
      Piece checking_piece = PieceAt(checking_piece_square).value();

      // We're being checked by exactly one piece. There are three options
      // available to us:
      //   1. Capture the checking piece.
      //   2. Block the checking piece.
      //   3. Move the king out of danger.
      if (moving_piece->kind() != kKing) {
        // If we're moving to a piece other than where the checking piece
        // resides, our intention is to block the checking piece.
        if (mov.Destination() != checking_piece_square && !mov.IsEnPassant()) {
          Bitboard all_pieces_with_block = Pieces(kWhite) | Pieces(kBlack);
          all_pieces_with_block.Unset(mov.Source());
          all_pieces_with_block.Set(mov.Destination());
          if (checking_piece
                  .Attacks(checking_piece_square, all_pieces_with_block)
                  .Test(king)) {
            return false;
          }
        } else if (!mov.IsEnPassant()) {
          CHECK(mov.IsCapture())
              << "moving to checking piece square but not a capture";
        } else {
          // TODO: en passant check
        }
      }
    }
  }

  // At this point we have validated that the candidate move gets us out of
  // check, if we are in check. Next we must validate that the candidate move
  // doesn't put is back in check.
  //
  // This can happen in three ways:
  //   1. The piece being moved is a king and the king moved into check
  //   2. A piece that was absolutely pinned moved
  //   3. An en-passant move captured a piece, removing two pieces from the same
  //   rank and leaving an attack on the king.

  // If we're not in check, we're fine as long as we don't move into check.
  // If this is a king move, does it move into check?
  if (moving_piece->kind() == kKing &&
      !SquaresAttacking(!SideToMove(), mov.Destination()).Empty()) {
    return false;
  }

  // Is this piece absolutely pinned to the king?
  if (IsAbsolutelyPinned(!SideToMove(), mov.Source())) {
    return false;
  }

  // TODO(sean): For en-passant moves, we have to do a special check: an
  // en-passant capture can take two pawns off a single rank, which can place
  // the king in check if something is attacking that rank.
  return true;
}

void Position::MakeMove(Move mov) {
  // MakeMove operates by maintaining a stack of the data that is irreversibly
  // lost whenever a move is made and maintaining a single copy of the data
  // that is reversibly lost.
  //
  // Before making a move, we must first copy the current irreversible state
  // onto the stack. Record the move that we're about to make, so we can
  // replay it backwards later.
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
    zobrist::ModifySideToMove(current_state_.zobrist_hash_);
    if (side_to_move_ == kWhite) {
      current_state_.fullmove_clock++;
    }
    return;
  }

  auto moving_piece = PieceAt(mov.Source());
  CHECK(moving_piece.has_value()) << "no piece at move source square";

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
    // the move stack (unmaking a move), we'll look at the previous move's
    // entry to determine what piece was captured.
    //
    // Note that this requires there to be at least one irreverisble state
    // already on the stack, but that's guaranteed because it's impossible for
    // the first move of a game of chess to be a capture.
    irreversible_state_.top().last_capture_ = captured_piece->kind();
    RemovePiece(target_square);
  }

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
        << "rook not at destination: " << AsFen() << " " << mov;
    RemovePiece(rook_square);
    AddPiece(new_rook_square, *rook);
  }

  Piece piece_to_add = *moving_piece;
  if (mov.IsPromotion()) {
    piece_to_add = Piece(side_to_move_, mov.PromotionPiece());
  }

  RemovePiece(mov.Source());
  AddPiece(mov.Destination(), piece_to_add);
  if (mov.IsDoublePawnPush()) {
    // Double-pawn pushes set the en passant square.
    Direction ep_dir =
        SideToMove() == kWhite ? kDirectionSouth : kDirectionNorth;
    Square ep_sq = util::Towards(mov.Destination(), ep_dir);
    zobrist::ModifyEnPassant(current_state_.zobrist_hash_,
                             current_state_.en_passant_square, ep_sq);
    current_state_.en_passant_square = ep_sq;
  } else {
    zobrist::ModifyEnPassant(current_state_.zobrist_hash_,
                             current_state_.en_passant_square, {});
    current_state_.en_passant_square = {};
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
      CastleStatus mask = side_to_move_ == kWhite ? kCastleWhiteQueenside
                                                  : kCastleBlackQueenside;
      current_state_.castle_status &= ~mask;
      zobrist::ModifyQueensideCastle(current_state_.zobrist_hash_,
                                     side_to_move_);
    }

    if (CanCastleKingside(side_to_move_) && mov.Source() == kingside_rook) {
      // Move of the kingside rook. Can't castle kingside anymore.
      CastleStatus mask =
          side_to_move_ == kWhite ? kCastleWhiteKingside : kCastleBlackKingside;
      current_state_.castle_status &= ~mask;
      zobrist::ModifyKingsideCastle(current_state_.zobrist_hash_,
                                    side_to_move_);
    }
  } else if (moving_piece->kind() == kKing) {
    // Moving a king invalidates the castle on both sides.
    CastleStatus mask = side_to_move_ == kWhite ? kCastleWhite : kCastleBlack;
    current_state_.castle_status &= ~mask;
    zobrist::ModifyKingsideCastle(current_state_.zobrist_hash_, side_to_move_);
    zobrist::ModifyQueensideCastle(current_state_.zobrist_hash_, side_to_move_);
  }

  side_to_move_ = !side_to_move_;
  zobrist::ModifySideToMove(current_state_.zobrist_hash_);
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
  // To unmake the move, we must first restore the previous move's
  // irreversible state and then undo the reversible aspects of the move.
  CHECK(!irreversible_state_.empty()) << "no moves to unmake";
  current_state_ = irreversible_state_.top();
  irreversible_state_.pop();

  // See comment at the bottom of this function.
  uint64_t hack_hash = current_state_.zobrist_hash_;

  // Reverse the side to move.
  side_to_move_ = !side_to_move_;

  CHECK(current_state_.move.has_value()) << "no move available to unmake";
  Move mov = *current_state_.move;

  // The rest of UnmakeMove proceeds in reverse of MakeMove; find the piece at
  // the destination square, remove it, replace it with the piece that was
  // captured, and move the piece back to the source.
  //
  // Since "the piece that was captured" is irreverisble state, we grab that
  // from current state (which we just restored).
  auto moved_piece = PieceAt(mov.Destination());
  CHECK(moved_piece.has_value()) << "no piece at move destination square";

  RemovePiece(mov.Destination());
  if (mov.IsCapture()) {
    CHECK(current_state_.last_capture_.has_value())
        << "unmaking capture with no last capture piece";

    Square captured_piece_square = mov.Destination();
    if (mov.IsEnPassant()) {
      // Like in MakeMove, en passant is the only move where we have to put
      // the piece back somewhere other than the move destination.
      Direction ep_dir =
          side_to_move_ == kWhite ? kDirectionSouth : kDirectionNorth;
      captured_piece_square = util::Towards(mov.Destination(), ep_dir);
    }

    AddPiece(captured_piece_square,
             Piece(!side_to_move_, *current_state_.last_capture_));
  }

  Piece piece_to_add = *moved_piece;
  if (mov.IsPromotion()) {
    // Only pawns can be promoted, therefore the piece that this used to be is
    // a pawn.
    piece_to_add = Piece(side_to_move_, kPawn);
  }
  AddPiece(mov.Source(), piece_to_add);

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

  // HACK HACK HACK: the zobrist hash is saved in the irreversible state buffer
  // because it's hard to reverse it in UnmakeMove. We just called a bunch of
  // functions that messed with the hash, so just restore the hash from the
  // irreversible state buffer now.
  //
  // This sucks, but it works.
  current_state_.zobrist_hash_ = hack_hash;
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
