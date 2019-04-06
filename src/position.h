#pragma once

#include <array>
#include <cstdint>
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
 *
 * In practice, a Position contains:
 *   1. A number of bitboards that, when taken together, represent the location
 * of all pieces on the board.
 *   2. A stack of "irreversible state", or state that can't be inferred based
 * on the current state. This include things like the halfmove clock, which
 * resets to zero and it's impossible to know what its value was before a move.
 *   3. The player whose turn it is to move.
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

  /**
   * Returns the player to move.
   */
  Color SideToMove() const { return this->side_to_move_; }

  /**
   * Returns the current value of the fullmove clock.
   */
  int FullmoveClock() const { return this->current_state_.fullmove_clock; }

  /**
   * Returns the current value of the halfmove clock.
   */
  int HalfmoveClock() const { return this->current_state_.halfmove_clock; }

  /**
   * Returns the Zobrist hash of this position.
   */
  uint64_t ZobristHash() const { return current_state_.zobrist_hash_; }

  /**
   * Returns the current en passant square, if an en passant move is legal from
   * the current board position.
   */
  std::optional<Square> EnPassantSquare() const {
    return this->current_state_.en_passant_square;
  }

  /**
   * Returns whether or not the given color is allowed to castle kingside.
   *
   * @param color The side to query castle status for.
   */
  bool CanCastleKingside(Color color) const {
    CastleStatus mask =
        color == kWhite ? kCastleWhiteKingside : kCastleBlackKingside;
    return (this->current_state_.castle_status & mask) == mask;
  }

  /**
   * Returns whether or not the given color is allowed to castle queenside.
   *
   * @param color The side to query castle status for.
   */
  bool CanCastleQueenside(Color color) const {
    CastleStatus mask =
        color == kWhite ? kCastleWhiteQueenside : kCastleBlackQueenside;
    return (this->current_state_.castle_status & mask) == mask;
  }

  /**
   * Returns a bitboard of squares containing pieces of the given kind belonging
   * to the given color.
   *
   * @param color The color to query pieces for.
   * @param kind The requested piece kind.
   */
  Bitboard Pieces(Color color, PieceKind kind) const {
    int offset = color == kWhite ? 0 : 6;
    return boards_by_piece_[offset + static_cast<int>(kind)];
  }

  /**
   * Returns a bitboard of squares containing all pieces belonging to the given
   * color.
   *
   * @param color The color to query pieces for.
   */
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

  // Move legality
  bool IsLegalCheck(Move mov) const;

  Bitboard SquareAttacks(Square sq) const;

  struct IrreversibleInformation {
    std::optional<Move> move;
    std::optional<PieceKind> last_capture_;
    std::optional<Square> en_passant_square;
    int halfmove_clock;
    int fullmove_clock;
    CastleStatus castle_status;
    uint64_t zobrist_hash_;
  };

  IrreversibleInformation current_state_;
  std::stack<IrreversibleInformation> irreversible_state_;
  std::array<Bitboard, 12> boards_by_piece_;
  std::array<Bitboard, 2> boards_by_color_;
  Color side_to_move_;
};

inline std::ostream& operator<<(std::ostream& os, const Position& pos) {
  os << std::endl;
  pos.Dump(os);
  return os;
}

}  // namespace apollo
