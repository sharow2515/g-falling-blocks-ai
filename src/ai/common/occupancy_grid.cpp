#include "ai/common/occupancy_grid.hpp"

#include <cassert>

namespace fb::ai::common {

OccupancyGrid::OccupancyGrid(const core::Board& board)
    : width_(board.Width()), height_(board.TotalHeight()), visibleHeight_(board.Height()) {
    assert(width_ <= 32 && "OccupancyGrid packs each row into a uint32_t; widen the row type if width exceeds 32");
    assert(height_ <= kMaxRows && "OccupancyGrid uses a fixed-size row buffer; raise kMaxRows if boards grow taller");

    for (int y = 0; y < height_; ++y) {
        std::uint32_t row = 0;
        for (int x = 0; x < width_; ++x) {
            if (board.CellColor({x, y}).has_value()) {
                row |= (1u << x);
            }
        }
        rows_[static_cast<std::size_t>(y)] = row;
    }
}

}  // namespace fb::ai::common
