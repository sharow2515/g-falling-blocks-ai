#pragma once

#include <vector>

#include "ai/common/occupancy_grid.hpp"
#include "core/types.hpp"

namespace fb::ai::common {

// 着地したピースの中心の高さ((minY+maxY)/2)。cellsは配置時のワールド座標。
double ComputeLandingHeight(const std::vector<core::Vec2i>& cells);

// (消去ライン数) x (消去に寄与したcellsのセル数)。
// cellsは配置時のワールド座標(消去前)、clearedRowsはそのときの消去行番号のリスト。
double ComputeErodedPieceCells(const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows);

// 上にブロックがある空きセルの数。
int CountHoles(const OccupancyGrid& grid);

// 各行の空/埋の切り替わり回数の総和(壁は埋扱い)。
int CountRowTransitions(const OccupancyGrid& grid);

// 各列の空/埋の切り替わり回数の総和(床は埋扱い、天井は扱わない)。
int CountColumnTransitions(const OccupancyGrid& grid);

// 井戸(両側が埋まった縦の空き)の深さdごとにd(d+1)/2を加算した合計。
int CountCumulativeWells(const OccupancyGrid& grid);

}  // namespace fb::ai::common
