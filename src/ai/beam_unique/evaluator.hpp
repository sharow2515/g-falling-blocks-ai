#pragma once

#include <string>
#include <vector>

#include "ai/common/occupancy_grid.hpp"
#include "core/types.hpp"

namespace fb::ai::beam_unique {

struct Features {
    double landingHeight = 0.0;
    double erodedPieceCells = 0.0;
    double rowTransitions = 0.0;
    double columnTransitions = 0.0;
    double holes = 0.0;
    double cumulativeWells = 0.0;
    double bumpiness = 0.0;
    double aggregateHeight = 0.0;
    double holeDepth = 0.0;

    bool causesGameOver = false;
};

struct Weights {
    double landingHeight = -4.222872604628931;
    double erodedPieceCells = 0.5267286904410413;
    double rowTransitions = -9.981653421474142;
    double columnTransitions = -4.244974414578996;
    double holes = -4.064196036562922;
    double cumulativeWells = -2.874873002269033;
    double bumpiness = -0.816394344335359;
    double aggregateHeight = -3.343403961839296;
    double holeDepth = -2.486832413166647;
};

double Evaluate(const Features& features, const Weights& weights = Weights{});

Weights ParseWeights(const std::string& arg);

Features ComputeFeaturesFromClearedBoard(const common::OccupancyGrid& clearedBoard,
                                          const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows);

}  // namespace fb::ai::beam_unique
