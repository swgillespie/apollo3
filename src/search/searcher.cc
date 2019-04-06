#include <limits>

#include "log.h"
#include "searcher.h"

namespace apollo::search {

SearchResult Searcher::Search(Position& pos, int depth) {
  nodes_ = 0;
  Move best_move = Move::Null();
  double best_score = -std::numeric_limits<double>::infinity();
  double alpha = best_score;
  double beta = -best_score;
  for (Move mov : pos.PseudolegalMoves()) {
    pos.MakeMove(mov);
    double score = -AlphaBeta(pos, -beta, -alpha, depth - 1);
    pos.UnmakeMove();
    if (score > alpha) {
      alpha = score;
    }
    if (score > best_score) {
      best_score = score;
      best_move = mov;
    }
  }

  return {best_move, best_score, nodes_};
}

double Searcher::AlphaBeta(Position& pos, double alpha, double beta,
                           int depth) {
  if (depth == 0) {
    return Quiesce(pos, alpha, beta);
  }

  for (Move mov : pos.PseudolegalMoves()) {
    pos.MakeMove(mov);
    double score = -AlphaBeta(pos, -beta, -alpha, depth - 1);
    pos.UnmakeMove();
    if (score >= beta) {
      return beta;
    }
    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}

double Searcher::Quiesce(Position& pos, double alpha, double beta) {
  nodes_++;
  double value = evaluator_->Evaluate(pos);
  return pos.SideToMove() == kBlack ? -value : value;
}

}  // namespace apollo::search