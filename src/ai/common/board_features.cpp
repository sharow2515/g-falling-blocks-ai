#include "ai/common/board_features.hpp"

#include <algorithm>
#include <limits>

namespace fb::ai::common {

double ComputeLandingHeight(const std::vector<core::Vec2i>& cells) {
    int minRow = std::numeric_limits<int>::max();
    int maxRow = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        minRow = std::min(minRow, c.y);
        maxRow = std::max(maxRow, c.y);
    }
    return (minRow + maxRow) / 2.0;
}

double ComputeErodedPieceCells(const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows) {
    int erodedCellCount = 0;
    for (int row : clearedRows) {
        for (const auto& c : cells) {
            if (c.y == row) ++erodedCellCount;
        }
    }
    return static_cast<double>(clearedRows.size()) * static_cast<double>(erodedCellCount);
}

int CountHoles(const OccupancyGrid& grid) {
    int holes = 0;
    for (int x = 0; x < grid.Width(); ++x) {
        bool seenFilled = false;
        for (int y = grid.VisibleHeight() - 1; y >= 0; --y) {
            bool occupied = grid.IsOccupied({x, y});
            if (occupied) {
                seenFilled = true;
            } else if (seenFilled) {
                ++holes;
            }
        }
    }
    return holes;
}

int CountRowTransitions(const OccupancyGrid& grid) {
    int transitions = 0;
    for (int y = 0; y < grid.VisibleHeight(); ++y) {
        bool prevFilled = true;  // 左の壁は「埋」扱い
        for (int x = 0; x <= grid.Width(); ++x) {
            bool filled = (x == grid.Width()) ? true : grid.IsOccupied({x, y});
            if (filled != prevFilled) ++transitions;
            prevFilled = filled;
        }
    }
    return transitions;
}

int CountColumnTransitions(const OccupancyGrid& grid) {
    int transitions = 0;
    for (int x = 0; x < grid.Width(); ++x) {
        bool prevFilled = true;  // 床は「埋」扱い
        for (int y = 0; y < grid.VisibleHeight(); ++y) {
            bool filled = grid.IsOccupied({x, y});
            if (filled != prevFilled) ++transitions;
            prevFilled = filled;
        }
        // 天井は壁扱いしない(上には積み増せるため)。
    }
    return transitions;
}

int CountCumulativeWells(const OccupancyGrid& grid) {
    int cumulative = 0;
    for (int x = 0; x < grid.Width(); ++x) {
        int depth = 0;
        for (int y = grid.VisibleHeight() - 1; y >= 0; --y) {
            bool filled = grid.IsOccupied({x, y});
            bool leftFilled = (x == 0) ? true : grid.IsOccupied({x - 1, y});
            bool rightFilled = (x == grid.Width() - 1) ? true : grid.IsOccupied({x + 1, y});

            if (!filled && leftFilled && rightFilled) {
                ++depth;
                cumulative += depth;
            } else {
                depth = 0;
            }
        }
    }
    return cumulative;
}

}  // namespace fb::ai::common
