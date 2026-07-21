#pragma once

#include <vector>

#include "core/board.hpp"
#include "core/types.hpp"

namespace fb::ai::reachability {

struct Features {
    double landingHeight = 0.0;
    double erodedPieceCells = 0.0;
    double rowTransitions = 0.0;
    double columnTransitions = 0.0;
    double holes = 0.0;
    double cumulativeWells = 0.0;
};

double Evaluate(const Features& features);
Features ComputeFeatures(core::Board board, const std::vector<core::Vec2i>& cells, core::Color color);
double ScorePlacement(const core::Board& board, const std::vector<core::Vec2i>& cells, core::Color color);

}  // namespace fb::ai::reachability
