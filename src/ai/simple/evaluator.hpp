#pragma once

#include "ai/simple/placement.hpp"
#include "core/board.hpp"

namespace fb::ai {

double ScorePlacement(const core::Board& board, const Placement& placement);

}  // namespace fb::ai
