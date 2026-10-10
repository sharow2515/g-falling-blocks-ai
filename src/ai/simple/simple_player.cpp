#include "ai/simple/simple_player.hpp"

#include "ai/simple/evaluator.hpp"
#include "ai/simple/placement.hpp"
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
    const std::vector<core::Vec2i>& newCells = mino.GetCells(newRotation);

    core::KickTableType kickType = core::SelectKickTable(mino.name, mino.gridSize);
    const std::vector<core::Vec2i>& kicks = core::GetWallKicks(kickType, state.rotation, newRotation);

    for (const auto& kick : kicks) {
        core::Vec2i newPos = state.position + kick;
        if (board.IsValidPosition(newCells, newPos)) {
            return RotationState{newRotation, newPos};
        }
    }
    return state;
}

}  // namespace

std::vector<PlayerMove> SimplePlayer::Plan(const core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const core::Mino* mino = game.GetActivePiece().MinoPtr();
    if (mino == nullptr) return {};

    std::vector<Placement> placements = EnumeratePlacements(game.GetBoard(), *mino);
    if (placements.empty()) return {};

    // 最初の候補を無条件でbestにしておく(全候補がゲームオーバー行きで
    // 同点の-infinityになる場合でも、どれか1つは選んで手詰まりを避ける)。
    const Placement* best = &placements.front();
    double bestScore = ScorePlacement(game.GetBoard(), *best, mino->color);
    for (std::size_t i = 1; i < placements.size(); ++i) {
        double score = ScorePlacement(game.GetBoard(), placements[i], mino->color);
        if (score > bestScore) {
            bestScore = score;
            best = &placements[i];
        }
    }

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
