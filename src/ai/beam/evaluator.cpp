#include "ai/beam/evaluator.hpp"

#include <limits>

#include "ai/common/board_features.hpp"
#include "ai/common/occupancy_grid.hpp"

namespace fb::ai::beam {

double Evaluate(const Features& f) {
    if (f.causesGameOver) return -std::numeric_limits<double>::infinity();

    return -1.0 * f.landingHeight + 1.0 * f.erodedPieceCells - 1.0 * f.rowTransitions -
           1.0 * f.columnTransitions - 4.0 * f.holes - 1.0 * f.cumulativeWells;
}

Features ComputeFeaturesFromClearedBoard(const core::Board& clearedBoard, const std::vector<core::Vec2i>& cells,
                                          const std::vector<int>& clearedRows) {
    Features f;
    f.landingHeight = common::ComputeLandingHeight(cells);
    f.erodedPieceCells = common::ComputeErodedPieceCells(cells, clearedRows);

    common::OccupancyGrid grid(clearedBoard);
    f.holes = common::CountHoles(grid);
    f.rowTransitions = common::CountRowTransitions(grid);
    f.columnTransitions = common::CountColumnTransitions(grid);
    f.cumulativeWells = common::CountCumulativeWells(grid);
    f.causesGameOver = clearedBoard.IsGameOver();
    return f;
}

}  // namespace fb::ai::beam
