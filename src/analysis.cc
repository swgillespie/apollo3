#include <algorithm>

#include "analysis.h"
#include "log.h"

namespace apollo {

Bitboard Analysis::DoubledPawns(Color color) {
  Bitboard pawns = pos_.Pawns(color);
  Bitboard answer;
  for (Bitboard file_bb : kBBFiles) {
    Bitboard pawns_on_file = file_bb & pawns;
    if (pawns_on_file.Count() > 1) {
      answer = answer | pawns_on_file;
    }
  }
  return answer;
}

Bitboard Analysis::BackwardPawns(Color color) {
  Bitboard pawns = pos_.Pawns(color);
  Bitboard answer;
  for (File file : kFiles) {
    Bitboard adjacent_files = AdjacentFiles(file);
    Bitboard current_file = kBBFiles[file];
    Bitboard pawns_on_current_file = pawns & current_file;
    Bitboard pawns_on_adjacent_files = pawns & adjacent_files;
    if (pawns_on_current_file.Empty()) {
      continue;
    }

    auto walk_rank = [&](auto begin, auto end) {
      for (auto it = begin; it != end; it++) {
        Bitboard current_file_rank = *it & pawns_on_current_file;
        Bitboard adjacent_file_rank = *it & pawns_on_adjacent_files;
        if (!current_file_rank.Empty() && adjacent_file_rank.Empty()) {
          answer = answer | current_file_rank;
          break;
        }

        if (!adjacent_file_rank.Empty() && current_file_rank.Empty()) {
          break;
        }
      }
    };

    if (color == kWhite) {
      walk_rank(std::begin(kBBRanks), std::end(kBBRanks));
    } else {
      walk_rank(std::rbegin(kBBRanks), std::rend(kBBRanks));
    }
  }

  return answer;
}

Bitboard Analysis::IsolatedPawns(Color color) {
  Bitboard pawns = pos_.Pawns(color);
  Bitboard answer;
  for (File file : kFiles) {
    Bitboard adjacent_files = AdjacentFiles(file);
    Bitboard current_file = kBBFiles[file];
    Bitboard pawns_on_current_file = pawns & current_file;
    Bitboard pawns_on_adjacent_files = pawns & adjacent_files;
    if (pawns_on_current_file.Empty()) {
      continue;
    }

    if (pawns_on_adjacent_files.Empty()) {
      answer = answer | pawns_on_current_file;
    }
  }

  return answer;
}

int Analysis::Mobility(Color color) {
  std::vector<Move> pseudolegal_moves = pos_.PseudolegalMoves();
  int move_count = 0;
  for (Move mov : pseudolegal_moves) {
    if (pos_.IsLegalGivenPseudolegal(mov)) {
      move_count++;
    }
  }
  return move_count;
}

Bitboard Analysis::AdjacentFiles(File file) {
  switch (file) {
    case kFileA:
      return kBBFileB;
    case kFileB:
      return kBBFileA | kBBFileC;
    case kFileC:
      return kBBFileB | kBBFileD;
    case kFileD:
      return kBBFileC | kBBFileE;
    case kFileE:
      return kBBFileD | kBBFileF;
    case kFileF:
      return kBBFileE | kBBFileG;
    case kFileG:
      return kBBFileF | kBBFileH;
    case kFileH:
      return kBBFileG;
    default:
      CHECK(false) << "invalid file";
      return Bitboard();
  }
}

}  // namespace apollo
