#pragma once

#include <vector>

#include "ai/common/occupancy_grid.hpp"
#include "ai/player.hpp"
#include "core/board.hpp"
#include "core/mino.hpp"
#include "core/types.hpp"

namespace fb::ai::common {

struct ReachablePlacement {
    int rotation = 0;
    core::Vec2i position{};
    std::vector<core::Vec2i> cells;  // ボード座標系(ワールド座標)での最終セル一覧
    std::vector<PlayerMove> path;  // スポーン位置からの操作(Left/Right/Down/RotateCW/RotateCCWのみ)
};

// grid上でmino(spawnRotation, spawnPositionから開始)が、実際の移動・回転
// (ウォールキック含む)の組み合わせで到達しうる、これ以上下に動けない状態を
// 全て列挙する(BFS)。呼び出し元が既にOccupancyGridを持っている場合はこちらを直接使うこと。
std::vector<ReachablePlacement> SearchReachablePlacements(const OccupancyGrid& grid, const core::Mino& mino,
                                                            int spawnRotation, core::Vec2i spawnPosition);

// core::Boardから毎回OccupancyGridを作ってから上記に委譲するオーバーロード。
std::vector<ReachablePlacement> SearchReachablePlacements(const core::Board& board, const core::Mino& mino,
                                                            int spawnRotation, core::Vec2i spawnPosition);

}  // namespace fb::ai::common
