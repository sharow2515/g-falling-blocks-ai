#include "ai/heuristic/heuristic_player.hpp"

#include <limits>

#include "ai/heuristic/evaluator.hpp"
#include "ai/heuristic/placement.hpp"
#include "core/wall_kick.hpp"

namespace fb::ai {

namespace {

struct RotationState {
    int rotation;
    core::Vec2i position;
};

RotationState ApplyRotationStep(const core::Board& board, const core::Mino& mino, RotationState state,
                                 int direction) {
    int newRotation = ((state.rotation + direction) % 4 + 4) % 4;
    std::vector<core::Vec2i> newCells = mino.GetCells(newRotation);

    core::KickTableType kickType = core::SelectKickTable(mino.name, mino.gridSize);
    std::vector<core::Vec2i> kicks = core::GetWallKicks(kickType, state.rotation, newRotation);

    for (const auto& kick : kicks) {
        core::Vec2i newPos = state.position + kick;
        if (board.IsValidPosition(newCells, newPos)) {
            return RotationState{newRotation, newPos};
        }
    }
    return state;
}

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

void HeuristicPlayer::Update(core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;
    lastSeenSpawnEventId_ = spawnId;

    for (PlayerMove move : BuildPlan(game)) {
        ExecuteMove(game, move);
    }
}

std::vector<PlayerMove> HeuristicPlayer::Plan(core::Game& game) { return BuildPlan(game); }

std::vector<PlayerMove> HeuristicPlayer::BuildPlan(core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const core::Mino* mino = game.GetActivePiece().MinoPtr();
    if (mino == nullptr) return {};

    std::vector<Placement> placements = EnumeratePlacements(game.GetBoard(), *mino);
    if (placements.empty()) return {};

    double bestScore = -std::numeric_limits<double>::infinity();
    const Placement* best = nullptr;
    for (const auto& p : placements) {
        double score = ScorePlacement(game.GetBoard(), p, mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &p;
        }
    }
    if (best == nullptr) return {};

    std::vector<PlayerMove> plan;

    // 回転はウォールキックでxがずれる可能性があるため先に(シミュレートして)決め、
    // 移動量はそのシミュレート結果を見てから決める。
    // 近い方向(時計回り/反時計回りの少ない方)を選ぶことで、途中の回転状態で
    // キックが1つも成立せず立ち往生するリスクを減らす。
    RotationState state{game.GetActivePiece().Rotation(), game.GetActivePiece().Position()};
    int cwSteps = ((best->rotation - state.rotation) % 4 + 4) % 4;

    if (cwSteps <= 2) {
        for (int i = 0; i < cwSteps; ++i) {
            state = ApplyRotationStep(game.GetBoard(), *mino, state, 1);
            plan.push_back(PlayerMove::RotateCW);
        }
    } else {
        state = ApplyRotationStep(game.GetBoard(), *mino, state, -1);
        plan.push_back(PlayerMove::RotateCCW);
    }

    int deltaX = best->x - state.position.x;
    while (deltaX > 0) {
        plan.push_back(PlayerMove::Right);
        --deltaX;
    }
    while (deltaX < 0) {
        plan.push_back(PlayerMove::Left);
        ++deltaX;
    }

    plan.push_back(PlayerMove::HardDrop);
    return plan;
}

}  // namespace fb::ai
