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
    CHECK(util::RankOf(pawn) != promo_rank)
        << "Pawns shouldn't be on the promotion rank";
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
        CHECK(!allied_pieces.Test(target))
            << "Square can't be occupied by both allied and enemy pieces";
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
    if (pos.EnPassantSquare()) {
      Square ep_square = *pos.EnPassantSquare();
      // Would this move be a normal legal attack for this pawn?
      if (attacks::PawnAttacks(pawn, color).Test(ep_square)) {
        // If so, the attack square is directly behind the pawn that was pushed.
        Square attack_sq = util::Towards(ep_square, ep_dir);
        CHECK(enemy_pieces.Test(attack_sq))
            << "square behind EP-square unoccupied";
        CHECK(!pieces.Test(ep_square)) << "EP-square should be unoccupied";
        moves.push_back(Move::EnPassant(pawn, ep_square));
      }
    }
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

void GenerateKingMoves(const Position& pos, std::vector<Move>& moves) {
  Color color = pos.SideToMove();
  Bitboard enemy_pieces = pos.Pieces(!color);
  Bitboard allied_pieces = pos.Pieces(color);
  pos.Kings(color).ForEach([&](Square king) {
    attacks::KingAttacks(king).ForEach([&](Square target) {
      if (enemy_pieces.Test(target)) {
        moves.push_back(Move::Capture(king, target));
      } else if (!allied_pieces.Test(target)) {
        moves.push_back(Move::Quiet(king, target));
      }
    });

    if (pos.IsCheck(color)) {
      // No castling out of check.
      return;
    }

    Bitboard all_pieces = allied_pieces | enemy_pieces;
    if (pos.CanCastleKingside(color)) {
      Square one = util::Towards(king, kDirectionEast);
      Square two = util::Towards(one, kDirectionEast);
      if (!all_pieces.Test(one) && !all_pieces.Test(two)) {
        // The king moves across both squares one and two and it is illegal to
        // castle through check. We can only proceed if no enemy piece is
        // attacking the squares the king travels upon.
        if (pos.SquaresAttacking(!color, one).Empty() &&
            pos.SquaresAttacking(!color, two).Empty()) {
          moves.push_back(Move::KingsideCastle(king, two));
        }
      }
    }

    if (pos.CanCastleQueenside(color)) {
      Square one = util::Towards(king, kDirectionWest);
      Square two = util::Towards(one, kDirectionWest);
      Square three = util::Towards(two, kDirectionWest);
      if (!all_pieces.Test(one) && !all_pieces.Test(two) &&
          !all_pieces.Test(three)) {
        // Square three can be checked, but it can't be occupied. The rook
        // travels across square three, but the king does not.
        if (pos.SquaresAttacking(!color, one).Empty() &&
            pos.SquaresAttacking(!color, two).Empty()) {
          moves.push_back(Move::QueensideCastle(king, two));
        }
      }
    }
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
  GenerateKingMoves(pos, moves);
}

}  // namespace movegen

}  // namespace apollo
