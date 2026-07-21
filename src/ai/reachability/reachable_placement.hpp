#pragma once

#include <vector>

#include "core/board.hpp"
#include "core/mino.hpp"
#include "core/types.hpp"

namespace fb::ai::reachability {

enum class MoveType {
    Left,
    Right,
    Down,
    RotateCW,
    RotateCCW,
};

struct ReachablePlacement {
    int rotation = 0;
    core::Vec2i position{};
    std::vector<core::Vec2i> cells;  // ボード座標系(ワールド座標)での最終セル一覧
    std::vector<MoveType> path;
};

std::vector<ReachablePlacement> SearchReachablePlacements(const core::Board& board, const core::Mino& mino,
                                                            int spawnRotation, core::Vec2i spawnPosition);

}  // namespace fb::ai::reachability
