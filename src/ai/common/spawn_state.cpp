#include "ai/common/spawn_state.hpp"

#include <limits>
#include <vector>

namespace fb::ai::common {

std::optional<SpawnState> ComputeSpawnState(const OccupancyGrid& board, const core::Mino& mino) {
    const std::vector<core::Vec2i>& cells = mino.GetCells(0);

    int maxCellY = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        if (c.y > maxCellY) maxCellY = c.y;
    }

    int spawnX = board.Width() / 2 - mino.gridSize / 2;
    int spawnY = board.VisibleHeight() - maxCellY - 1;
    core::Vec2i pos{spawnX, spawnY};

    if (!board.IsValidPosition(cells, pos)) {
        pos.y += 1;
        if (!board.IsValidPosition(cells, pos)) {
            pos.y += 1;
            if (!board.IsValidPosition(cells, pos)) {
                return std::nullopt;
            }
        }
    }

    return SpawnState{0, pos};
}

}  // namespace fb::ai::common
