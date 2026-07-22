#pragma once

#include <vector>

#include "core/board.hpp"
#include "core/types.hpp"

namespace fb::ai::beam {

// ai::reachability::evaluator と同じ6特徴量。
struct Features {
    double landingHeight = 0.0;
    double erodedPieceCells = 0.0;
    double rowTransitions = 0.0;
    double columnTransitions = 0.0;
    double holes = 0.0;
    double cumulativeWells = 0.0;
};

// 重み付き線形和。landingHeight=-1, erodedPieceCells=+1, rowTransitions=-1,
// columnTransitions=-1, holes=-4, cumulativeWells=-1
double Evaluate(const Features& features);

// 既にcellsを配置してライン消去まで済ませたclearedBoardから特徴量を計算する。
// cellsは配置時のワールド座標(消去前)、clearedRowsはそのときの消去行番号のリスト。
Features ComputeFeaturesFromClearedBoard(const core::Board& clearedBoard, const std::vector<core::Vec2i>& cells,
                                          const std::vector<int>& clearedRows);

}  // namespace fb::ai::beam
