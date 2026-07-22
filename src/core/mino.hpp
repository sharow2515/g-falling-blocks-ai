#pragma once

#include <array>
#include <string>
#include <vector>

#include "core/types.hpp"

namespace fb::core {

struct Mino {
    int id = -1;
    std::string name;
    Color color;
    bool isOriginal = false;
    int gridSize = 3;
    std::vector<Vec2i> baseCells;  // 回転0の状態のセル座標(x,y)。x,y は [0, gridSize-1]。

    // 指定回転状態(0=0°,1=90°,2=180°,3=270°)のセル座標を返す。
    // 結果は初回呼び出し時にこのMino内でキャッシュされ、以後は使い回される
    // (baseCells/gridSizeはロード後不変なので、回転ごとの結果も不変)。
    [[nodiscard]] const std::vector<Vec2i>& GetCells(int rotation) const;

private:
    [[nodiscard]] Vec2i RotateCell(Vec2i cell, int rotation) const;

    mutable std::array<std::vector<Vec2i>, 4> cellsByRotation_;
    mutable std::array<bool, 4> cellsComputed_{};
};

}  // namespace fb::core
