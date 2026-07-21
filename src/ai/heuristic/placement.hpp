#pragma once

#include <vector>

#include "core/board.hpp"
#include "core/mino.hpp"

namespace fb::ai {

// 現ミノの1つの「回転×最終x位置」の着地パターン。
struct Placement {
    int rotation = 0;
    int x = 0;
    int y = 0;
    std::vector<core::Vec2i> cells;  // ボード座標系(ワールド座標)でのセル一覧
};

// board 上で mino が取りうる全ての着地パターンを列挙する。
// ウォールキックは考慮しない(回転後の形状を素直に横スライド+落下させるだけ)。
std::vector<Placement> EnumeratePlacements(const core::Board& board, const core::Mino& mino);

}  // namespace fb::ai
