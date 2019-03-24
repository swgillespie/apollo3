#include "movegen.h"
#include "attacks.h"
#include "util.h"

namespace apollo {

namespace {

void GeneratePawnMoves(const Position& pos, std::vector<Move>& moves) {
  Color color = pos.SideToMove();
  Bitboard enemy_pieces = pos.Pieces(!color);
  Bitboard allied_pieces = pos.Pieces(color);
  Bitboard pieces = enemy_pieces | allied_pieces;

  Rank start_rank = color == kWhite ? kRank2 : kRank7;
  Rank promo_rank = color == kWhite ? kRank8 : kRank1;
  Direction pawn_dir = color == kWhite ? kDirectionNorth : kDirectionSouth;
  Direction ep_dir = color == kWhite ? kDirectionSouth : kDirectionNorth;

  pos.Pawns(color).ForEach([&](Square pawn) {
    // Pawns shouldn't be on the promotion rank.
    assert(util::RankOf(pawn) != promo_rank);
    Square target = util::Towards(pawn, pawn_dir);

    // Non-capturing moves.
    if (!pieces.Test(target)) {
      if (util::RankOf(target) == promo_rank) {
        moves.push_back(Move::Promotion(pawn, target, kKnight));
        moves.push_back(Move::Promotion(pawn, target, kBishop));
        moves.push_back(Move::Promotion(pawn, target, kRook));
        moves.push_back(Move::Promotion(pawn, target, kQueen));
      } else {
        moves.push_back(Move::Quiet(pawn, target));
      }
    }

    // Double pawn pushes, for pawns originating on the starting rank.
    if (util::RankOf(pawn) == start_rank) {
      Square two_push_target = util::Towards(target, pawn_dir);
      if (!pieces.Test(target) && !pieces.Test(two_push_target)) {
        moves.push_back(Move::DoublePawnPush(pawn, two_push_target));
      }
    }

    // Non-en-passant capturing moves.
    attacks::PawnAttacks(pawn, color).ForEach([&](Square target) {
      if (enemy_pieces.Test(target)) {
        assert(!allied_pieces.Test(target));
        if (util::RankOf(target) == promo_rank) {
          moves.push_back(Move::PromotionCapture(pawn, target, kKnight));
          moves.push_back(Move::PromotionCapture(pawn, target, kBishop));
          moves.push_back(Move::PromotionCapture(pawn, target, kRook));
          moves.push_back(Move::PromotionCapture(pawn, target, kQueen));
        } else {
          moves.push_back(Move::Capture(pawn, target));
        }
      }
    });

    // En passant moves.
    // TODO
  });
}

}  // anonymous namespace

namespace movegen {

void GeneratePseudolegalMoves(const Position& pos, std::vector<Move>& moves) {
  GeneratePawnMoves(pos, moves);
}

}  // namespace movegen

}  // namespace apollo