#pragma once

#include <array>
#include <exception>
#include <iostream>
#include <optional>
#include <stack>
#include <string_view>
#include <vector>

#include "bitboard.h"
#include "move.h"
#include "piece.h"
#include "types.h"

namespace apollo {

constexpr const char* kFenDefaultPosition =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum FenParseError {
  kInvalidDigit = 1,
  kFileInvalidSum,
  kUnexpectedEOF,
  kUnexpectedChar
};

class InvalidFenException : std::exception {
 public:
  explicit InvalidFenException(FenParseError result) : err_(result) {}

  const char* what() const noexcept override { return "invalid FEN"; }

 private:
  FenParseError err_;
};

/**
 * A Position represents a singular chess position. It contains all of the state
 * necessary to represent a instant in a game of chess, as well as enough state
 * to recreate all positions of the game up until this point.
 */
class Position {
 public:
  /**
   * Constructs a default Position, the default start position for a game of
   * chess.
   */
  Position() : Position(kFenDefaultPosition) {}

  /**
   * Constructs a new Position by parsing the given FEN string.
   */
  explicit Position(std::string_view fen);

  /**
   * Disallow copying of Positions.
   */
  Position(const Position&) = delete;

  /**
   * Disallow copying of Positions.
   */
  Position& operator=(const Position&) = delete;

  Color SideToMove() const { return this->side_to_move_; }

  int FullmoveClock() const { return this->current_state_.fullmove_clock; }

  int HalfmoveClock() const { return this->current_state_.halfmove_clock; }

  std::optional<Square> EnPassantSquare() const {
    return this->current_state_.en_passant_square;
  }

  bool CanCastleKingside(Color color) const {
    CastleStatus mask =
        color == kWhite ? kCastleWhiteKingside : kCastleBlackKingside;
    return (this->current_state_.castle_status & mask) == mask;
  }

  bool CanCastleQueenside(Color color) const {
    CastleStatus mask =
        color == kWhite ? kCastleWhiteQueenside : kCastleBlackQueenside;
    return (this->current_state_.castle_status & mask) == mask;
  }

  Bitboard Pieces(Color color, PieceKind kind) const {
    int offset = color == kWhite ? 0 : 6;
    return boards_by_piece_[offset + static_cast<int>(kind)];
  }

  Bitboard Pieces(Color color) const {
    return boards_by_color_[color == kWhite ? 0 : 1];
  }

  Bitboard Pawns(Color color) const { return Pieces(color, kPawn); }
  Bitboard Knights(Color color) const { return Pieces(color, kKnight); }
  Bitboard Bishops(Color color) const { return Pieces(color, kBishop); }
  Bitboard Rooks(Color color) const { return Pieces(color, kRook); }
  Bitboard Queens(Color color) const { return Pieces(color, kQueen); }
  Bitboard Kings(Color color) const { return Pieces(color, kKing); }

  std::string AsFen() const;

  Bitboard SquaresAttacking(Color to_move, Square sq) const;
  bool IsCheck(Color to_move) const;

  // Pin detection
  bool IsAbsolutelyPinned(Color to_move, Square sq) const;

  // Move legality testing
  bool IsLegal(Move mov) const;

  void AddPiece(Square sq, Piece piece);
  void RemovePiece(Square sq);
  std::optional<Piece> PieceAt(Square sq) const;

  void MakeMove(Move mov);
  void UnmakeMove();

  std::vector<Move> PseudolegalMoves() const;

  void Dump(std::ostream& out) const;

 private:
  friend class FenParser;

  struct IrreversibleInformation {
    std::optional<Move> move;
    std::optional<PieceKind> last_capture_;
    std::optional<Square> en_passant_square;
    int halfmove_clock;
    int fullmove_clock;
    CastleStatus castle_status;
  };

  IrreversibleInformation current_state_;
  std::stack<IrreversibleInformation> irreversible_state_;
  std::array<Bitboard, 12> boards_by_piece_;
  std::array<Bitboard, 2> boards_by_color_;
  Color side_to_move_;
};

}  // namespace apollo
