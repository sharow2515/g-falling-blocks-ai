#include "ai/common/rotation_cache.hpp"

#include <algorithm>

namespace fb::ai::common {

void WarmUpRotationCaches(std::vector<const core::Mino*> minos) {
    std::sort(minos.begin(), minos.end());
    minos.erase(std::unique(minos.begin(), minos.end()), minos.end());
    for (const core::Mino* mino : minos) {
        for (int rotation = 0; rotation < 4; ++rotation) static_cast<void>(mino->GetCells(rotation));
    }
}

}  // namespace fb::ai::common
