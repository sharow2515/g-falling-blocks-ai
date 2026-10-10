#include "ai/simple/evaluator.hpp"

#include <limits>
#include <vector>

#include "ai/common/board_features.hpp"
#include "ai/common/occupancy_grid.hpp"

namespace fb::ai {

double ScorePlacement(const core::Board& board, const Placement& placement) {
    core::Board after = board;
    after.PlacePiece(placement.cells, core::Vec2i{0, 0}, core::Color{});  // 色は採点に使わない

    std::vector<int> clearedRows;
    after.ClearFullLines(clearedRows);

    if (after.IsGameOver()) return -std::numeric_limits<double>::infinity();

    const common::OccupancyGrid grid(after);
    const double landingHeight = common::ComputeLandingHeight(placement.cells);
    const double erodedPieceCells = common::ComputeErodedPieceCells(placement.cells, clearedRows);

    return -1.0 * landingHeight + 1.0 * erodedPieceCells - 1.0 * common::CountRowTransitions(grid) -
           1.0 * common::CountColumnTransitions(grid) - 4.0 * common::CountHoles(grid) -
           1.0 * common::CountCumulativeWells(grid);
}

}  // namespace fb::ai
