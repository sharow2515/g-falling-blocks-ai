#include "core/mino.hpp"

namespace fb::core {

std::vector<Vec2i> Mino::GetCells(int rotation) const {
    std::vector<Vec2i> rotated;
    rotated.reserve(baseCells.size());
    for (const auto& cell : baseCells) {
        rotated.push_back(RotateCell(cell, rotation));
    }
    return rotated;
}

Vec2i Mino::RotateCell(Vec2i cell, int rotation) const {
    rotation = ((rotation % 4) + 4) % 4;

    Vec2i result = cell;
    for (int i = 0; i < rotation; ++i) {
        // 90°時計回り回転: (x, y) -> (y, gridSize-1-x)
        int newX = result.y;
        int newY = gridSize - 1 - result.x;
        result = Vec2i{newX, newY};
    }
    return result;
}

}  // namespace fb::core
