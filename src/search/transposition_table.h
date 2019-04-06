#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "board_evaluator.h"

namespace apollo::search {

enum TranspositionTableEntryKind { kTTExact, kTTBeta, kTTAlpha };

struct TranspositionTableEntry {
  uint64_t zobrist_key;
  uint16_t depth;
  double score;
  std::optional<Move> best_move;
  TranspositionTableEntryKind kind;
};

class TranspositionTable {
 public:
  TranspositionTable();

  void Insert(uint64_t key, uint16_t depth, double score,
              TranspositionTableEntryKind kind);

  TranspositionTableEntry* Find(uint64_t key);

 private:
  std::unordered_map<uint64_t, TranspositionTableEntry> table_;
};

}  // namespace apollo::search
