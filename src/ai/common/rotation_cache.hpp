#pragma once

#include <vector>

#include "core/mino.hpp"

namespace fb::ai::common {

void WarmUpRotationCaches(std::vector<const core::Mino*> minos);

}  // namespace fb::ai::common
