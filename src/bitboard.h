#pragma once

#include <cassert>
#include <cstdint>
#include "types.h"

namespace apollo {

class BitboardIterator {
 public:
  explicit BitboardIterator(uint64_t bits) : bits_(bits) {}

  bool HasNext() const { return this->bits_ != 0; }

  Square Next() {
    assert(this->bits_ != 0);
    uint8_t next = __builtin_ctzl(this->bits_);
    this->bits_ &= this->bits_ - 1;
    return static_cast<Square>(next);
  }

 private:
  uint64_t bits_;
};

/**
 * A Bitboard is a set of squares on the chessboard. Since there are exactly 64
 * squares on a chessboard, a bitboard represents a complete set of squares in a
 * 64-bit integer.
 */
class Bitboard {
 public:
  /**
   * Creates a new, empty bitboard.
   */
  constexpr Bitboard() : bits_(0) {}

  /**
   * Creates a new, empty bitboard with the given bit pattern. A bit is 1 if a
   * particular square is a member of the set.
   *
   * @param bits The bit pattern to use for the bitboard.
   */
  constexpr explicit Bitboard(uint64_t bits) : bits_(bits) {}

  constexpr bool Test(Square sq) const {
    return (this->bits_ & (1ULL << static_cast<int>(sq))) != 0;
  }

  constexpr void Set(Square sq) {
    this->bits_ |= (1ULL << static_cast<int>(sq));
  }

  constexpr void Unset(Square sq) {
    this->bits_ &= ~(1ULL << static_cast<int>(sq));
  }

  constexpr bool Empty() const { return this->bits_ == 0; }

  BitboardIterator Iterator() const { return BitboardIterator(this->bits_); }

  constexpr Bitboard operator&(const Bitboard& other) {
    return Bitboard(this->bits_ & other.bits_);
  }

  constexpr Bitboard operator|(const Bitboard& other) {
    return Bitboard(this->bits_ | other.bits_);
  }

  constexpr Bitboard operator^(const Bitboard& other) {
    return Bitboard(this->bits_ ^ other.bits_);
  }

 private:
  uint64_t bits_;
};

constexpr Bitboard BB_RANK_1 = Bitboard(0x00000000000000FFULL);
constexpr Bitboard BB_RANK_2 = Bitboard(0x000000000000FF00ULL);
constexpr Bitboard BB_RANK_3 = Bitboard(0x0000000000FF0000ULL);
constexpr Bitboard BB_RANK_4 = Bitboard(0x00000000FF000000ULL);
constexpr Bitboard BB_RANK_5 = Bitboard(0x000000FF00000000ULL);
constexpr Bitboard BB_RANK_6 = Bitboard(0x0000FF0000000000ULL);
constexpr Bitboard BB_RANK_7 = Bitboard(0x00FF000000000000ULL);
constexpr Bitboard BB_RANK_8 = Bitboard(0xFF00000000000000ULL);

};  // namespace apollo
