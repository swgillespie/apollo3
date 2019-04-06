#include "transposition_table.h"

namespace apollo::search {

TranspositionTable::TranspositionTable() : table_() {}

void TranspositionTable::Insert(uint64_t key, uint16_t depth, double score,
                                TranspositionTableEntryKind kind) {}

TranspositionTableEntry* Find(uint64_t key) { return nullptr; }

}  // namespace apollo::search