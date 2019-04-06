#include "searcher.h"
#include "log.h"

namespace apollo::search {

SearchResult Searcher::Search(Position& pos, int depth,
                              std::chrono::seconds time_budget) {
  LOG() << "beginning search of pos " << pos.AsFen();
  for (int depth = 1; depth < 10; depth++) {
    LOG() << "search depth: " << depth;
  }
}

double Searcher::AlphaBeta(Position& pos, double alpha, double beta,
                           int depth) {
  if (auto entry = transposition_table_.Find(pos.ZobristHash())) {
    return entry->score;
  }

  if (depth == 0) {
    return Quiesce(pos, alpha, beta);
  }

  TranspositionTableEntryKind entryKind = kTTAlpha;
  for (Move mov : pos.PseudolegalMoves()) {
    pos.MakeMove(mov);
    double score = -AlphaBeta(pos, -beta, -alpha, depth - 1);
    pos.UnmakeMove();
    if (score >= beta) {
      transposition_table_.Insert(pos.ZobristHash(), depth, beta, kTTBeta);
      return beta;
    }
    if (score > alpha) {
      entryKind = kTTExact;
      alpha = score;
    }
  }

  transposition_table_.Insert(pos.ZobristHash(), depth, alpha, entryKind);
  return alpha;
}

double Searcher::Quiesce(Position& pos, double alpha, double beta) {
  double value = evaluator_->Evaluate(pos);
  transposition_table_.Insert(pos.ZobristHash(), 0, value, kTTExact);
  return value;
}

}  // namespace apollo::search