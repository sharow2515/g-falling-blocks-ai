#pragma once

#include <string>
#include <vector>

#include "ai/common/occupancy_grid.hpp"
#include "core/types.hpp"

namespace fb::ai::beam_unique {

// beam_tunedと同じ9特徴量(評価関数自体は変えず、探索側の重複排除だけを変える)。
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

    // その配置でゲームオーバーになるかどうか
    // trueの場合、Evaluate()は他の指標に関係なく-infinityを返す。
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

// featuresとweightsの重み付き線形和。
// ただしcausesGameOverがtrueなら、他の選択肢がある限り絶対に選ばれないよう-infinityを返す。
double Evaluate(const Features& features, const Weights& weights = Weights{});

// "landingHeight,erodedPieceCells,rowTransitions,columnTransitions,holes,cumulativeWells,bumpiness,
// aggregateHeight,holeDepth" の順でカンマ区切り9個のdoubleをパースする。
// 不正な場合はstd::invalid_argumentを投げる。
Weights ParseWeights(const std::string& arg);

// 既にcellsを配置してライン消去まで済ませたclearedBoardから特徴量を計算する。
// cellsは配置時のワールド座標(消去前)、clearedRowsはそのときの消去行番号のリスト。
Features ComputeFeaturesFromClearedBoard(const common::OccupancyGrid& clearedBoard,
                                          const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows);

}  // namespace fb::ai::beam_unique
