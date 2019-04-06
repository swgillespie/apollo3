#pragma once

#include <array>
#include <iostream>

namespace apollo {

enum Square {
  A1,
  B1,
  C1,
  D1,
  E1,
  F1,
  G1,
  H1,
  A2,
  B2,
  C2,
  D2,
  E2,
  F2,
  G2,
  H2,
  A3,
  B3,
  C3,
  D3,
  E3,
  F3,
  G3,
  H3,
  A4,
  B4,
  C4,
  D4,
  E4,
  F4,
  G4,
  H4,
  A5,
  B5,
  C5,
  D5,
  E5,
  F5,
  G5,
  H5,
  A6,
  B6,
  C6,
  D6,
  E6,
  F6,
  G6,
  H6,
  A7,
  B7,
  C7,
  D7,
  E7,
  F7,
  G7,
  H7,
  A8,
  B8,
  C8,
  D8,
  E8,
  F8,
  G8,
  H8,
  kSquareLast,
};

constexpr std::array<Square, kSquareLast> kSquares = {
    A1, B1, C1, D1, E1, F1, G1, H1, A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3, A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5, A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7, A8, B8, C8, D8, E8, F8, G8, H8,
};

enum Rank {
  kRank1,
  kRank2,
  kRank3,
  kRank4,
  kRank5,
  kRank6,
  kRank7,
  kRank8,
  kRankLast,
};

constexpr std::array<Rank, kRankLast> kRanks = {
    kRank1, kRank2, kRank3, kRank4, kRank5, kRank6, kRank7, kRank8,
};

enum File {
  kFileA,
  kFileB,
  kFileC,
  kFileD,
  kFileE,
  kFileF,
  kFileG,
  kFileH,
  kFileLast
};

constexpr std::array<File, kFileLast> kFiles = {
    kFileA, kFileB, kFileC, kFileD, kFileE, kFileF, kFileG, kFileH,
};

enum Color { kWhite, kBlack, kColorLast };

inline Color operator!(Color c) { return c == kWhite ? kBlack : kWhite; }
inline std::ostream& operator<<(std::ostream& os, Color c) {
  if (c == kWhite) {
    os << "w";
  } else if (c == kBlack) {
    os << "b";
  } else {
    os << "?";
  }
  return os;
}

constexpr std::array<Color, kColorLast> kColors = { kWhite, kBlack };

enum PieceKind {
  kPieceFirst = -1,
  kPawn,
  kKnight,
  kBishop,
  kRook,
  kQueen,
  kKing,
  kPieceLast,
};

constexpr std::array<PieceKind, kPieceLast> kPieces = {
    kPawn, kKnight, kBishop, kRook, kQueen, kKing,
};

inline std::ostream& operator<<(std::ostream& os, PieceKind pk) {
  switch (pk) {
    case kPawn:
      os << "p";
      break;
    case kKnight:
      os << "n";
      break;
    case kBishop:
      os << "b";
      break;
    case kRook:
      os << "r";
      break;
    case kQueen:
      os << "q";
      break;
    case kKing:
      os << "k";
      break;
    default:
      os << "?";
      break;
  }
  return os;
}

enum CastleStatus : int {
  kCastleNone = 0x00,
  kCastleWhiteKingside = 0x01,
  kCastleWhiteQueenside = 0x02,
  kCastleWhite = 0x03,
  kCastleBlackKingside = 0x04,
  kCastleBlackQueenside = 0x08,
  kCastleBlack = 0x0C,
  kCastleAll = 0x0F
};

inline CastleStatus operator|(CastleStatus lhs, CastleStatus rhs) {
  return static_cast<CastleStatus>(static_cast<int>(lhs) |
                                   static_cast<int>(rhs));
}

inline CastleStatus operator&(CastleStatus lhs, CastleStatus rhs) {
  return static_cast<CastleStatus>(static_cast<int>(lhs) &
                                   static_cast<int>(rhs));
}

inline CastleStatus& operator|=(CastleStatus& lhs, CastleStatus rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline CastleStatus& operator&=(CastleStatus& lhs, CastleStatus rhs) {
  lhs = lhs & rhs;
  return lhs;
}

inline CastleStatus operator~(CastleStatus op) {
  return static_cast<CastleStatus>(~static_cast<int>(op));
}

enum Direction {
  kDirectionNorth,
  kDirectionNorthEast,
  kDirectionEast,
  kDirectionSouthEast,
  kDirectionSouth,
  kDirectionSouthWest,
  kDirectionWest,
  kDirectionNorthWest,
  kDirectionLast
};

constexpr std::array<int, kDirectionLast> kDirectionVectors = {8,  9,  1,  -7,
                                                               -8, -9, -1, 7};

}  // namespace apollo
