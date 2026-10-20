#pragma once

#include <vector>

#include "ai/player_move.hpp"

namespace fb::ai::common {

// 経路末尾の連続するDownを取り除き、最後に必ずHardDropを1つ追加する。
std::vector<PlayerMove> ToExecutablePlan(const std::vector<PlayerMove>& path);

}  // namespace fb::ai::common
