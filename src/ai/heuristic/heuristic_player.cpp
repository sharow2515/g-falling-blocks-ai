#include "ai/heuristic/heuristic_player.hpp"

#include <limits>

#include "ai/heuristic/evaluator.hpp"
#include "ai/heuristic/placement.hpp"

namespace fb::ai {

void HeuristicPlayer::Update(core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;
    lastSeenSpawnEventId_ = spawnId;

    if (game.IsGameOver() || game.IsPaused()) return;
    if (!game.GetActivePiece().IsActive()) return;

    PlayBestMove(game);
}

void HeuristicPlayer::PlayBestMove(core::Game& game) {
    const core::Mino* mino = game.GetActivePiece().MinoPtr();
    if (mino == nullptr) return;

    std::vector<Placement> placements = EnumeratePlacements(game.GetBoard(), *mino);
    if (placements.empty()) return;

    double bestScore = -std::numeric_limits<double>::infinity();
    const Placement* best = nullptr;
    for (const auto& p : placements) {
        double score = ScorePlacement(game.GetBoard(), p, mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &p;
        }
    }
    if (best == nullptr) return;

    // 回転はウォールキックでxがずれる可能性があるため先に行い、
    // 移動量は回転後の実際の位置を見てから決める。
    // 近い方向(時計回り/反時計回りの少ない方)を選ぶことで、途中の回転状態で
    // キックが1つも成立せず立ち往生するリスクを減らす。
    int currentRotation = game.GetActivePiece().Rotation();
    int cwSteps = ((best->rotation - currentRotation) % 4 + 4) % 4;
    if (cwSteps <= 2) {
        for (int i = 0; i < cwSteps; ++i) {
            game.RotateCW();
        }
    } else {
        game.RotateCCW();
    }

    int currentX = game.GetActivePiece().Position().x;
    int deltaX = best->x - currentX;
    while (deltaX > 0) {
        game.MoveRight();
        --deltaX;
    }
    while (deltaX < 0) {
        game.MoveLeft();
        ++deltaX;
    }

    game.HardDrop();
}

}  // namespace fb::ai
