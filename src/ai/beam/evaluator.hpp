#pragma once

#include <string>
#include <vector>

#include "ai/common/occupancy_grid.hpp"
#include "core/types.hpp"

namespace fb::ai::beam {

struct Features {
    double landingHeight = 0.0;
    double erodedPieceCells = 0.0;
    double rowTransitions = 0.0;
    double columnTransitions = 0.0;
    double holes = 0.0;
    double cumulativeWells = 0.0;

    bool causesGameOver = false;
};

struct Weights {
    double landingHeight = -1.0;
    double erodedPieceCells = 1.0;
    double rowTransitions = -1.0;
    double columnTransitions = -1.0;
    double holes = -4.0;
    double cumulativeWells = -1.0;
};

double Evaluate(const Features& features, const Weights& weights = Weights{});

Weights ParseWeights(const std::string& arg);

Features ComputeFeaturesFromClearedBoard(const common::OccupancyGrid& clearedBoard,
                                          const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows);

}  // namespace fb::ai::beam
