#include "ai/reachability/reachability_player.hpp"

#include <limits>

#include "ai/reachability/evaluator.hpp"
#include "ai/reachability/reachable_placement.hpp"

namespace fb::ai::reachability {

void ReachabilityPlayer::Update(core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;
    lastSeenSpawnEventId_ = spawnId;

    if (game.IsGameOver() || game.IsPaused()) return;
    if (!game.GetActivePiece().IsActive()) return;

    PlayBestMove(game);
}

void ReachabilityPlayer::PlayBestMove(core::Game& game) {
    const core::Mino* mino = game.GetActivePiece().MinoPtr();
    if (mino == nullptr) return;

    int spawnRotation = game.GetActivePiece().Rotation();
    core::Vec2i spawnPosition = game.GetActivePiece().Position();

    std::vector<ReachablePlacement> placements =
        SearchReachablePlacements(game.GetBoard(), *mino, spawnRotation, spawnPosition);
    if (placements.empty()) return;

    double bestScore = -std::numeric_limits<double>::infinity();
    const ReachablePlacement* best = nullptr;
    for (const auto& p : placements) {
        double score = ScorePlacement(game.GetBoard(), p.cells, mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &p;
        }
    }
    if (best == nullptr) return;

    // 経路末尾に連続するDownは省く。
    std::vector<MoveType> path = best->path;
    while (!path.empty() && path.back() == MoveType::Down) {
        path.pop_back();
    }

    // 経路を、そのまま同期的に(1回のUpdate呼び出し内で)再生する。
    // この関数の実行中は core::Game::Tick() が絶対に挟まらないことが前提
    // (呼ばれてしまうと自由落下で盤面がズレ、探索結果と実際の着地先が食い違う)。
    for (MoveType move : path) {
        switch (move) {
            case MoveType::Left: game.MoveLeft(); break;
            case MoveType::Right: game.MoveRight(); break;
            case MoveType::Down: game.MoveDown(); break;
            case MoveType::RotateCW: game.RotateCW(); break;
            case MoveType::RotateCCW: game.RotateCCW(); break;
        }
    }

    game.HardDrop();
}

}  // namespace fb::ai::reachability
