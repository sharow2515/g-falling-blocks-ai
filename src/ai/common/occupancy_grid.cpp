#include "ai/common/occupancy_grid.hpp"

#include <cassert>

namespace fb::ai::common {

OccupancyGrid::OccupancyGrid(const core::Board& board)
    : width_(board.Width()), height_(board.TotalHeight()), visibleHeight_(board.Height()) {
    assert(width_ <= kMaxCols && "OccupancyGrid packs each row into a uint32_t; widen the row type if width exceeds 32");
    assert(height_ <= kMaxRows && "OccupancyGrid uses a fixed-size row buffer; raise kMaxRows if boards grow taller");
    assert(height_ <= 32 &&
           "cols_は列ごとに全高(TotalHeight)分のビットを1つのuint32_tに収めているため、高さが32を超えられない");

    for (int y = 0; y < height_; ++y) {
        std::uint32_t row = 0;
        for (int x = 0; x < width_; ++x) {
            if (board.CellColor({x, y}).has_value()) {
                row |= (1u << x);
                cols_[static_cast<std::size_t>(x)] |= (1u << y);
            }
        }
        rows_[static_cast<std::size_t>(y)] = row;
    }
}

}  // namespace fb::ai::common
