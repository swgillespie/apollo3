#pragma once

#include <optional>

#include "piece.h"
#include "types.h"

namespace apollo::util {

template <typename R, typename F>
Square SquareOf(R rank, F file) {
  return static_cast<Square>(static_cast<int>(rank) * 8 +
                             static_cast<int>(file));
}

inline Square Towards(Square sq, Direction dir) {
  return static_cast<Square>(static_cast<int>(sq) + kDirectionVectors[dir]);
}

inline Rank RankOf(Square sq) {
  return static_cast<Rank>(static_cast<int>(sq) >> 3);
}

inline std::optional<Rank> CharToRank(char c) {
  switch (c) {
    case '1':
      return kRank1;
    case '2':
      return kRank2;
    case '3':
      return kRank3;
    case '4':
      return kRank4;
    case '5':
      return kRank5;
    case '6':
      return kRank6;
    case '7':
      return kRank7;
    case '8':
      return kRank8;
    default:
      return {};
  }
}

inline std::optional<File> CharToFile(char c) {
  switch (c) {
    case 'a':
      return kFileA;
    case 'b':
      return kFileB;
    case 'c':
      return kFileC;
    case 'd':
      return kFileD;
    case 'e':
      return kFileE;
    case 'f':
      return kFileF;
    case 'g':
      return kFileG;
    case 'h':
      return kFileH;
    default:
      return {};
  }
}

}  // namespace apollo::util
