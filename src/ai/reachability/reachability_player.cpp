#include "ai/reachability/reachability_player.hpp"

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

    // 最初の候補を無条件でbestにしておく(全候補がゲームオーバー行きで
    // 同点の-infinityになる場合でも、どれか1つは選んで手詰まりを避ける)。
    const common::ReachablePlacement* best = &placements.front();
    double bestScore = ScorePlacement(game.GetBoard(), best->cells, mino->color);
    for (std::size_t i = 1; i < placements.size(); ++i) {
        double score = ScorePlacement(game.GetBoard(), placements[i].cells, mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &placements[i];
        }
    }

    return common::ConvertPath(best->path);
}

}  // namespace fb::ai::reachability
