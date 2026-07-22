#include "ai/reachability/reachability_player.hpp"

#include <limits>

#include "ai/common/move_execution.hpp"
#include "ai/common/path_conversion.hpp"
#include "ai/common/reachable_search.hpp"
#include "ai/reachability/evaluator.hpp"

namespace fb::ai::reachability {

void ReachabilityPlayer::Update(core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;
    lastSeenSpawnEventId_ = spawnId;

    // 判断から実行まで同期的に(1回のUpdate呼び出し内で)完結させる。
    // この関数の実行中は core::Game::Tick() が絶対に挟まらないことが前提
    // (呼ばれてしまうと自由落下で盤面がズレ、探索結果と実際の着地先が食い違う)。
    for (PlayerMove move : BuildPlan(game)) {
        common::ExecuteMove(game, move);
    }
}

std::vector<PlayerMove> ReachabilityPlayer::Plan(core::Game& game) { return BuildPlan(game); }

std::vector<PlayerMove> ReachabilityPlayer::BuildPlan(core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const core::Mino* mino = game.GetActivePiece().MinoPtr();
    if (mino == nullptr) return {};

    int spawnRotation = game.GetActivePiece().Rotation();
    core::Vec2i spawnPosition = game.GetActivePiece().Position();

    std::vector<common::ReachablePlacement> placements =
        common::SearchReachablePlacements(game.GetBoard(), *mino, spawnRotation, spawnPosition);
    if (placements.empty()) return {};

    double bestScore = -std::numeric_limits<double>::infinity();
    const common::ReachablePlacement* best = nullptr;
    for (const auto& p : placements) {
        double score = ScorePlacement(game.GetBoard(), p.cells, mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &p;
        }
    }
    if (best == nullptr) return {};

    return common::ConvertPath(best->path);
}

}  // namespace fb::ai::reachability
