#include "ai/beam_unique/evaluator.hpp"

#include <limits>
#include <sstream>
#include <stdexcept>

#include "ai/common/board_features.hpp"
#include "ai/common/occupancy_grid.hpp"

namespace fb::ai::beam_unique {

double Evaluate(const Features& f, const Weights& w) {
    if (f.causesGameOver) return -std::numeric_limits<double>::infinity();

    return w.landingHeight * f.landingHeight + w.erodedPieceCells * f.erodedPieceCells +
           w.rowTransitions * f.rowTransitions + w.columnTransitions * f.columnTransitions +
           w.holes * f.holes + w.cumulativeWells * f.cumulativeWells + w.bumpiness * f.bumpiness +
           w.aggregateHeight * f.aggregateHeight + w.holeDepth * f.holeDepth;
}

Weights ParseWeights(const std::string& arg) {
    std::vector<double> values;
    std::stringstream ss(arg);
    std::string token;
    while (std::getline(ss, token, ',')) {
        values.push_back(std::stod(token));
    }
    if (values.size() != 9) {
        throw std::invalid_argument("--weightsは9個のカンマ区切り数値である必要があります: " + arg);
    }

    Weights weights;
    weights.landingHeight = values[0];
    weights.erodedPieceCells = values[1];
    weights.rowTransitions = values[2];
    weights.columnTransitions = values[3];
    weights.holes = values[4];
    weights.cumulativeWells = values[5];
    weights.bumpiness = values[6];
    weights.aggregateHeight = values[7];
    weights.holeDepth = values[8];
    return weights;
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
    f.bumpiness = common::ComputeBumpiness(clearedBoard);
    f.aggregateHeight = common::ComputeAggregateHeight(clearedBoard);
    f.holeDepth = common::ComputeHoleDepth(clearedBoard);
    f.causesGameOver = clearedBoard.IsGameOver();
    return f;
}

}  // namespace fb::ai::beam_unique
