#pragma once

#include "ai/simple/placement.hpp"
#include "core/board.hpp"
#include "core/types.hpp"

namespace fb::ai {

// 盤面評価用の特徴量。現在の中身は Pierre Dellacherie の6特徴量。
struct SimpleFeatures {
    double landingHeight = 0.0;      // 着地したピースの中心の高さ
    double erodedPieceCells = 0.0;   // (消去ライン数) x (消去に寄与した今回のピースのセル数)
    double rowTransitions = 0.0;     // 各行の空/埋の切り替わり回数の総和(壁は埋扱い)
    double columnTransitions = 0.0;  // 各列の空/埋の切り替わり回数の総和(床は埋扱い、天井は扱わない)
    double holes = 0.0;              // 上にブロックがある空きセルの数
    double cumulativeWells = 0.0;    // 井戸(両側が埋まった縦の空き)の深さdごとにd(d+1)/2を加算した合計
};

// 重み付き線形和
// landingHeight=-1, erodedPieceCells=+1, rowTransitions=-1,
// columnTransitions=-1, holes=-4, cumulativeWells=-1
double Evaluate(const SimpleFeatures& features);

SimpleFeatures ComputeFeatures(core::Board board, const Placement& placement, core::Color color);

// ComputeFeatures + Evaluate
double ScorePlacement(const core::Board& board, const Placement& placement, core::Color color);

}  // namespace fb::ai
