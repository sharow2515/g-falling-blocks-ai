#include "ai/reachability/reachability_player.hpp"

#include "ai/common/executable_plan.hpp"
#include "ai/common/reachable_search.hpp"
#include "ai/reachability/evaluator.hpp"

namespace fb::ai::reachability {

std::vector<PlayerMove> ReachabilityPlayer::Plan(const core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const core::Mino& mino = *game.GetActivePiece().MinoPtr();

    int spawnRotation = game.GetActivePiece().Rotation();
    core::Vec2i spawnPosition = game.GetActivePiece().Position();

    std::vector<common::ReachablePlacement> placements =
        common::SearchReachablePlacements(game.GetBoard(), mino, spawnRotation, spawnPosition);
    if (placements.empty()) return {};

    const common::ReachablePlacement* best = &placements.front();
    double bestScore = ScorePlacement(game.GetBoard(), best->cells);
    for (std::size_t i = 1; i < placements.size(); ++i) {
        double score = ScorePlacement(game.GetBoard(), placements[i].cells);
        if (score > bestScore) {
            bestScore = score;
            best = &placements[i];
        }
    }

    return common::ToExecutablePlan(best->path);
}

}  // namespace fb::ai::reachability
