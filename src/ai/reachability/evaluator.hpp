#pragma once

#include <vector>

#include "core/board.hpp"
#include "core/types.hpp"

namespace fb::ai::reachability {

double ScorePlacement(const core::Board& board, const std::vector<core::Vec2i>& cells);

}  // namespace fb::ai::reachability
