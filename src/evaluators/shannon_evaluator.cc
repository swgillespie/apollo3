#include "shannon_evaluator.h"
#include "analysis.h"

namespace apollo::evaluators {

const double kKingWeight = 200;
const double kQueenWeight = 9;
const double kRookWeight = 5;
const double kBishopWeight = 3;
const double kKnightWeight = 3;
const double kPawnWeight = 1;
const double kPawnFormationWeight = 0.5;
const double kMobilityWeight = 0.1;

ShannonEvaluator::ShannonEvaluator() {}

double ShannonEvaluator::Evaluate(const Position& pos) const {
  Analysis boardAnalysis(pos);

  double kingScore =
      kKingWeight * (pos.Kings(kWhite).Count() - pos.Kings(kBlack).Count());
  double queenScore =
      kQueenWeight * (pos.Queens(kWhite).Count() - pos.Queens(kBlack).Count());
  double rookScore =
      kRookWeight * (pos.Rooks(kWhite).Count() - pos.Rooks(kBlack).Count());
  double bishopScore = kBishopWeight * (pos.Bishops(kWhite).Count() -
                                        pos.Bishops(kBlack).Count());
  double knightScore = kKnightWeight * (pos.Knights(kWhite).Count() -
                                        pos.Knights(kBlack).Count());
  double pawnScore =
      kPawnWeight * (pos.Pawns(kWhite).Count() - pos.Pawns(kBlack).Count());
  double mobilityScore = kMobilityWeight * (boardAnalysis.Mobility(kWhite) -
                                            boardAnalysis.Mobility(kBlack));
  double isolatedPawnScore =
      kPawnFormationWeight * (boardAnalysis.IsolatedPawns(kWhite).Count() -
                              boardAnalysis.IsolatedPawns(kBlack).Count());
  double backwardPawnScore =
      kPawnFormationWeight * (boardAnalysis.BackwardPawns(kWhite).Count() -
                              boardAnalysis.BackwardPawns(kBlack).Count());
  double doubledPawnScore =
      kPawnFormationWeight * (boardAnalysis.DoubledPawns(kWhite).Count() -
                              boardAnalysis.DoubledPawns(kBlack).Count());

  return kingScore + queenScore + rookScore + bishopScore + knightScore +
         pawnScore + isolatedPawnScore + backwardPawnScore + doubledPawnScore +
         mobilityScore;
}

}  // namespace apollo::evaluators
