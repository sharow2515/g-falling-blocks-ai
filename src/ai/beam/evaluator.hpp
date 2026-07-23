#pragma once

#include <vector>

#include "ai/common/occupancy_grid.hpp"
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

    // その配置でゲームオーバーになるかどうか
    // trueの場合、Evaluate()は他の指標に関係なく-infinityを返す。
    bool causesGameOver = false;
};

// 重み付き線形和。landingHeight=-1, erodedPieceCells=+1, rowTransitions=-1,
// columnTransitions=-1, holes=-4, cumulativeWells=-1
// ただしcausesGameOverがtrueなら、他の選択肢がある限り絶対に選ばれないよう-infinityを返す。
double Evaluate(const Features& features);

// 既にcellsを配置してライン消去まで済ませたclearedBoardから特徴量を計算する。
// cellsは配置時のワールド座標(消去前)、clearedRowsはそのときの消去行番号のリスト。
Features ComputeFeaturesFromClearedBoard(const common::OccupancyGrid& clearedBoard,
                                          const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows);

}  // namespace fb::ai::beam
