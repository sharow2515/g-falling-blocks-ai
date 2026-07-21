#include "ai/reachability/reachability_player.hpp"

#include <limits>

#include "ai/reachability/evaluator.hpp"
#include "ai/reachability/reachable_placement.hpp"

namespace fb::ai::reachability {

namespace {

void ExecuteMove(core::Game& game, PlayerMove move) {
    switch (move) {
        case PlayerMove::Left: game.MoveLeft(); break;
        case PlayerMove::Right: game.MoveRight(); break;
        case PlayerMove::Down: game.MoveDown(); break;
        case PlayerMove::RotateCW: game.RotateCW(); break;
        case PlayerMove::RotateCCW: game.RotateCCW(); break;
        case PlayerMove::HardDrop: game.HardDrop(); break;
    }
}

}  // namespace

void ReachabilityPlayer::Update(core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;
    lastSeenSpawnEventId_ = spawnId;

    // 判断から実行まで同期的に(1回のUpdate呼び出し内で)完結させる。
    // この関数の実行中は core::Game::Tick() が絶対に挟まらないことが前提
    // (呼ばれてしまうと自由落下で盤面がズレ、探索結果と実際の着地先が食い違う)。
    for (PlayerMove move : BuildPlan(game)) {
        ExecuteMove(game, move);
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

    std::vector<ReachablePlacement> placements =
        SearchReachablePlacements(game.GetBoard(), *mino, spawnRotation, spawnPosition);
    if (placements.empty()) return {};

    double bestScore = -std::numeric_limits<double>::infinity();
    const ReachablePlacement* best = nullptr;
    for (const auto& p : placements) {
        double score = ScorePlacement(game.GetBoard(), p.cells, mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &p;
        }
    }
    if (best == nullptr) return {};

    // 経路末尾に連続するDownは省く。
    std::vector<MoveType> path = best->path;
    while (!path.empty() && path.back() == MoveType::Down) {
        path.pop_back();
    }

    std::vector<PlayerMove> plan;
    plan.reserve(path.size() + 1);
    for (MoveType move : path) {
        switch (move) {
            case MoveType::Left: plan.push_back(PlayerMove::Left); break;
            case MoveType::Right: plan.push_back(PlayerMove::Right); break;
            case MoveType::Down: plan.push_back(PlayerMove::Down); break;
            case MoveType::RotateCW: plan.push_back(PlayerMove::RotateCW); break;
            case MoveType::RotateCCW: plan.push_back(PlayerMove::RotateCCW); break;
        }
    }
    plan.push_back(PlayerMove::HardDrop);
    return plan;
}

}  // namespace fb::ai::reachability
