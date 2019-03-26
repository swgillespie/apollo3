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

void GenerateKnightMoves(const Position& pos, std::vector<Move>& moves) {
  Color color = pos.SideToMove();
  Bitboard enemy_pieces = pos.Pieces(!color);
  Bitboard allied_pieces = pos.Pieces(color);
  pos.Knights(color).ForEach([&](Square knight) {
    attacks::KnightAttacks(knight).ForEach([&](Square target) {
      if (enemy_pieces.Test(target)) {
        moves.push_back(Move::Capture(knight, target));
      } else if (!allied_pieces.Test(target)) {
        moves.push_back(Move::Quiet(knight, target));
      }
    });
  });
}

template <typename BoardCallback, typename AttackCallback>
void GenerateSlidingMoves(const Position& pos, std::vector<Move>& moves,
                          BoardCallback bc, AttackCallback atk) {
  Color color = pos.SideToMove();
  Bitboard enemy_pieces = pos.Pieces(!color);
  Bitboard allied_pieces = pos.Pieces(color);
  bc(color).ForEach([&](Square piece) {
    atk(piece, enemy_pieces | allied_pieces).ForEach([&](Square target) {
      // In theory, we only need to test the end of rays for occupancy,
      // but this works.
      if (enemy_pieces.Test(target)) {
        moves.push_back(Move::Capture(piece, target));
      } else if (!allied_pieces.Test(target)) {
        moves.push_back(Move::Quiet(piece, target));
      }
    });
  });
}

}  // anonymous namespace

namespace movegen {

void GeneratePseudolegalMoves(const Position& pos, std::vector<Move>& moves) {
  GeneratePawnMoves(pos, moves);
  GenerateKnightMoves(pos, moves);
  GenerateSlidingMoves(pos, moves, [&](Color c) { return pos.Bishops(c); },
                       attacks::BishopAttacks);
  GenerateSlidingMoves(pos, moves, [&](Color c) { return pos.Rooks(c); },
                       attacks::RookAttacks);
  GenerateSlidingMoves(pos, moves, [&](Color c) { return pos.Queens(c); },
                       attacks::QueenAttacks);
}

}  // namespace movegen

}  // namespace apollo