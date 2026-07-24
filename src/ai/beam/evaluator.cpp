#include "ai/beam/evaluator.hpp"

#include <limits>

#include "ai/common/board_features.hpp"
#include "ai/common/occupancy_grid.hpp"

namespace fb::ai::beam {

double Evaluate(const Features& f, const Weights& w) {
    if (f.causesGameOver) return -std::numeric_limits<double>::infinity();

    return w.landingHeight * f.landingHeight + w.erodedPieceCells * f.erodedPieceCells +
           w.rowTransitions * f.rowTransitions + w.columnTransitions * f.columnTransitions +
           w.holes * f.holes + w.cumulativeWells * f.cumulativeWells;
}

Features ComputeFeaturesFromClearedBoard(const common::OccupancyGrid& clearedBoard,
                                          const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows) {
    Features f;
    f.landingHeight = common::ComputeLandingHeight(cells);
    f.erodedPieceCells = common::ComputeErodedPieceCells(cells, clearedRows);

    // clearedBoardは既にOccupancyGridなので、ここで作り直す必要はない。
    f.holes = common::CountHoles(clearedBoard);
    f.rowTransitions = common::CountRowTransitions(clearedBoard);
    f.columnTransitions = common::CountColumnTransitions(clearedBoard);
    f.cumulativeWells = common::CountCumulativeWells(clearedBoard);
    f.causesGameOver = clearedBoard.IsGameOver();
    return f;
}

}  // namespace fb::ai::beam
