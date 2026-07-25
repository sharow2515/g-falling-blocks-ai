#pragma once

#include <string>
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

struct Weights {
    double landingHeight = -1.0;
    double erodedPieceCells = 1.0;
    double rowTransitions = -1.0;
    double columnTransitions = -1.0;
    double holes = -4.0;
    double cumulativeWells = -1.0;
};

// featuresとweightsの重み付き線形和。
// ただしcausesGameOverがtrueなら、他の選択肢がある限り絶対に選ばれないよう-infinityを返す。
double Evaluate(const Features& features, const Weights& weights = Weights{});

// "landingHeight,erodedPieceCells,rowTransitions,columnTransitions,holes,cumulativeWells" の順で
// カンマ区切り6個のdoubleをパースする。不正な場合はstd::invalid_argumentを投げる。
Weights ParseWeights(const std::string& arg);

// 既にcellsを配置してライン消去まで済ませたclearedBoardから特徴量を計算する。
// cellsは配置時のワールド座標(消去前)、clearedRowsはそのときの消去行番号のリスト。
Features ComputeFeaturesFromClearedBoard(const common::OccupancyGrid& clearedBoard,
                                          const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows);

}  // namespace fb::ai::beam
