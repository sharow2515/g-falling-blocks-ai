#pragma once

#include <vector>

#include "ai/player_move.hpp"
#include "core/board.hpp"
#include "core/mino.hpp"

namespace fb::ai {

struct Placement {
    std::vector<core::Vec2i> cells;  // 盤面上の座標
    std::vector<PlayerMove> moves;
};

// 回転 → 左右移動 → ハードドロップ の順で操作して到達可能な場所とその操作列を列挙
std::vector<Placement> EnumerateReachablePlacements(const core::Board& board, const core::Mino& mino,
                                                    int spawnRotation, core::Vec2i spawnPosition);

}  // namespace fb::ai
