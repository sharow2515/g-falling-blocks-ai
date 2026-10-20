#include "ai/beam_tuned/evaluator.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

#include "ai/common/board_features.hpp"
#include "ai/common/occupancy_grid.hpp"

namespace fb::ai::beam_tuned {

namespace {

std::vector<std::string> SplitByComma(const std::string& text) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    while (true) {
        std::size_t comma = text.find(',', start);
        if (comma == std::string::npos) {
            tokens.push_back(text.substr(start));
            return tokens;
        }
        tokens.push_back(text.substr(start, comma - start));
        start = comma + 1;
    }
}

double ParseDouble(const std::string& token) {
    std::size_t pos = 0;
    try {
        double value = std::stod(token, &pos);
        if (pos == token.size() && std::isfinite(value)) return value;
    } catch (const std::exception&) {
    }
    throw std::invalid_argument("数値ではありません: '" + token + "'");
}

}  // namespace

double Evaluate(const Features& f, const Weights& w) {
    if (f.causesGameOver) return -std::numeric_limits<double>::infinity();

    return w.landingHeight * f.landingHeight + w.erodedPieceCells * f.erodedPieceCells +
           w.rowTransitions * f.rowTransitions + w.columnTransitions * f.columnTransitions +
           w.holes * f.holes + w.cumulativeWells * f.cumulativeWells + w.bumpiness * f.bumpiness +
           w.aggregateHeight * f.aggregateHeight + w.holeDepth * f.holeDepth;
}

Weights ParseWeights(const std::string& arg) {
    const std::vector<std::string> tokens = SplitByComma(arg);
    if (tokens.size() != 9) {
        throw std::invalid_argument("--weightsは9個のカンマ区切り数値である必要があります: " + arg);
    }

    std::vector<double> values;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        try {
            values.push_back(ParseDouble(tokens[i]));
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument("--weightsの" + std::to_string(i + 1) + "番目が不正です: " + e.what());
        }
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

}  // namespace fb::ai::beam_tuned
