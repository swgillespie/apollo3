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

    for (Bitboard rank : kBBRanks) {
      Bitboard current_file_rank = rank & pawns_on_current_file;
      Bitboard adjacent_file_rank = rank & pawns_on_adjacent_files;
      if (!current_file_rank.Empty() && !adjacent_file_rank.Empty()) {
        answer = answer | current_file_rank;
        break;
      }

      if (!adjacent_file_rank.Empty() && current_file_rank.Empty()) {
        break;
      }
    }
  }

  return answer;
}

Bitboard Analysis::IsolatedPawns(Color color) { return Bitboard(); }

int Analysis::Mobility(Color color) {
  std::vector<Move> pseudolegal_moves = pos_.PseudolegalMoves();
  int move_count = 0;
  for (Move mov : pseudolegal_moves) {
    if (pos_.IsLegal(mov)) {
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