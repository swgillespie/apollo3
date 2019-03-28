#pragma once

#include <cstdint>
#include <iostream>

#include "log.h"
#include "types.h"

namespace apollo {

class BitboardIterator {
 public:
  explicit BitboardIterator(uint64_t bits) : bits_(bits) {}

  bool HasNext() const { return this->bits_ != 0; }

  Square Next() {
    CHECK(this->bits_ != 0) << "called Next() when HasNext() is false";
    uint8_t next = __builtin_ctzll(this->bits_);
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

  constexpr uint64_t Bits() const { return this->bits_; }

  BitboardIterator Iterator() const { return BitboardIterator(this->bits_); }

  template <typename Callback>
  void ForEach(Callback cb) const {
    for (auto it = Iterator(); it.HasNext();) {
      Square sq = it.Next();
      cb(sq);
    }
  }

  void Dump(std::ostream& out) const {
    for (int rank = kRank8; rank >= kRank1; rank--) {
      for (int file = kFileA; file < kFileLast; file++) {
        Square sq = static_cast<Square>(rank * 8 + file);
        if (this->Test(sq)) {
          out << " 1 ";
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

  constexpr Bitboard operator&(const Bitboard& other) const {
    return Bitboard(this->bits_ & other.bits_);
  }

  constexpr Bitboard operator|(const Bitboard& other) const {
    return Bitboard(this->bits_ | other.bits_);
  }

  constexpr Bitboard operator^(const Bitboard& other) const {
    return Bitboard(this->bits_ ^ other.bits_);
  }

 private:
  uint64_t bits_;
};

constexpr Bitboard kBBRank1 = Bitboard(0x00000000000000FFULL);
constexpr Bitboard kBBRank2 = Bitboard(0x000000000000FF00ULL);
constexpr Bitboard kBBRank3 = Bitboard(0x0000000000FF0000ULL);
constexpr Bitboard kBBRank4 = Bitboard(0x00000000FF000000ULL);
constexpr Bitboard kBBRank5 = Bitboard(0x000000FF00000000ULL);
constexpr Bitboard kBBRank6 = Bitboard(0x0000FF0000000000ULL);
constexpr Bitboard kBBRank7 = Bitboard(0x00FF000000000000ULL);
constexpr Bitboard kBBRank8 = Bitboard(0xFF00000000000000ULL);

constexpr Bitboard kBBFileA = Bitboard(0x0101010101010101ULL);
constexpr Bitboard kBBFileB = Bitboard(0x0202020202020202ULL);
constexpr Bitboard kBBFileC = Bitboard(0x0404040404040404ULL);
constexpr Bitboard kBBFileD = Bitboard(0x0808080808080808ULL);
constexpr Bitboard kBBFileE = Bitboard(0x1010101010101010ULL);
constexpr Bitboard kBBFileF = Bitboard(0x2020202020202020ULL);
constexpr Bitboard kBBFileG = Bitboard(0x4040404040404040ULL);
constexpr Bitboard kBBFileH = Bitboard(0x8080808080808080ULL);

constexpr Bitboard kBBFileAB = kBBFileA | kBBFileB;
constexpr Bitboard kBBFileGH = kBBFileG | kBBFileH;

constexpr Bitboard kBBRank12 = kBBRank1 | kBBRank2;
constexpr Bitboard kBBRank78 = kBBRank7 | kBBRank8;

}  // namespace apollo
