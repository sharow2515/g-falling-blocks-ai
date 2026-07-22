#pragma once

#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai::common {

// 1手をcore::Gameに対して即座に実行する。
void ExecuteMove(core::Game& game, PlayerMove move);

}  // namespace fb::ai::common
