#pragma once

#include <optional>

#include "ai/common/occupancy_grid.hpp"
#include "core/mino.hpp"
#include "core/types.hpp"

namespace fb::ai::common {

struct SpawnState {
    int rotation;
    core::Vec2i position;
};

std::optional<SpawnState> ComputeSpawnState(const OccupancyGrid& board, const core::Mino& mino);

}  // namespace fb::ai::common
