#include "movegen.h"
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
    // TODO

    // Non-en-passant capturing moves.
    // TODO

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