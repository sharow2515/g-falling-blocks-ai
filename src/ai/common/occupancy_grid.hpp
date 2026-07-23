#pragma once

#include <cstdint>
#include <vector>

#include "core/board.hpp"
#include "core/types.hpp"

namespace fb::ai::common {

// core::Boardの占有状態を1回だけ読み取ってビットマスク化したスナップショット。
// 探索中にboard.CellColor()/IsValidPosition()を何度も呼ぶ代わりに、これを使い回す。
// 盤面は変化しない前提なので、関数ローカルでのみ使うこと(キャッシュ・保持はしない)。
class OccupancyGrid {
public:
    explicit OccupancyGrid(const core::Board& board);

    [[nodiscard]] bool IsInsideBoard(core::Vec2i pos) const {
        return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
    }

    [[nodiscard]] bool IsOccupied(core::Vec2i pos) const {
        return ((rows_[static_cast<std::size_t>(pos.y)] >> pos.x) & 1u) != 0;
    }

    [[nodiscard]] bool IsValidPosition(const std::vector<core::Vec2i>& cells, core::Vec2i position) const {
        for (const auto& cell : cells) {
            core::Vec2i boardPos = position + cell;
            if (!IsInsideBoard(boardPos)) return false;
            if (IsOccupied(boardPos)) return false;
        }
        return true;
    }

    [[nodiscard]] int Width() const { return width_; }
    [[nodiscard]] int Height() const { return height_; }

private:
    int width_;
    int height_;
    std::vector<std::uint32_t> rows_;  // 1行1ワード。bit x = 列x。行インデックス = y。
};

}  // namespace fb::ai::common
