#pragma once

#include <array>

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

typedef bool Color;
constexpr Color kWhite = true;
constexpr Color kBlack = false;

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

enum CastleStatus {
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
