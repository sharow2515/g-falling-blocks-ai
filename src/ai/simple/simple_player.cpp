#include "ai/simple/simple_player.hpp"

#include "ai/simple/evaluator.hpp"
#include "ai/simple/placement.hpp"

namespace fb::ai {

std::vector<PlayerMove> SimplePlayer::Plan(const core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const core::Mino& mino = *game.GetActivePiece().MinoPtr();

    std::vector<Placement> placements = EnumerateReachablePlacements(
        game.GetBoard(), mino, game.GetActivePiece().Rotation(), game.GetActivePiece().Position());
    if (placements.empty()) return {};

    const Placement* best = &placements.front();
    double bestScore = ScorePlacement(game.GetBoard(), *best);
    for (std::size_t i = 1; i < placements.size(); ++i) {
        double score = ScorePlacement(game.GetBoard(), placements[i]);
        if (score > bestScore) {
            bestScore = score;
            best = &placements[i];
        }
    }

    return best->moves;
}

}  // namespace fb::ai
