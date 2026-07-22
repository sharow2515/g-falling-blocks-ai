#include "ai/simple/placement.hpp"

#include <limits>

namespace fb::ai {

namespace {

// cellsをxだけ水平移動した状態で落下させ、最終的なyを返す。
// startYはcellsの最大y+startYがTotalHeight()-1に収まるよう呼び出し側で計算する。
int DropY(const core::Board& board, const std::vector<core::Vec2i>& cells, int x, int startY) {
    core::Vec2i pos{x, startY};

    int guard = board.TotalHeight() * 2;
    while (!board.IsValidPosition(cells, pos) && guard-- > 0) {
        pos.y -= 1;
    }

    while (board.IsValidPosition(cells, pos + core::Vec2i::Down())) {
        pos = pos + core::Vec2i::Down();
    }
    return pos.y;
}

}  // namespace

std::vector<Placement> EnumeratePlacements(const core::Board& board, const core::Mino& mino) {
    std::vector<Placement> placements;

    for (int rotation = 0; rotation < 4; ++rotation) {
        std::vector<core::Vec2i> cells = mino.GetCells(rotation);

        int minX = std::numeric_limits<int>::max();
        int maxX = std::numeric_limits<int>::min();
        int maxY = std::numeric_limits<int>::min();
        for (const auto& c : cells) {
            minX = std::min(minX, c.x);
            maxX = std::max(maxX, c.x);
            maxY = std::max(maxY, c.y);
        }

        int startY = board.TotalHeight() - 1 - maxY;
        int xLow = -minX;
        int xHigh = board.Width() - 1 - maxX;

        for (int x = xLow; x <= xHigh; ++x) {
            int y = DropY(board, cells, x, startY);
            core::Vec2i pos{x, y};
            if (!board.IsValidPosition(cells, pos)) continue;

            Placement p;
            p.rotation = rotation;
            p.x = x;
            p.y = y;
            p.cells.reserve(cells.size());
            for (const auto& c : cells) p.cells.push_back(c + pos);
            placements.push_back(std::move(p));
        }
    }

    return placements;
}

}  // namespace fb::ai
