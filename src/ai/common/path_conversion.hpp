#pragma once

#include <vector>

#include "ai/common/reachable_search.hpp"
#include "ai/player.hpp"

namespace fb::ai::common {

// BFS探索で得られた経路(MoveType列)を実行可能なPlayerMove列に変換する。
// 経路末尾の連続するDownは取り除く。
// 最後に必ずHardDropを1つ追加する。
std::vector<PlayerMove> ConvertPath(const std::vector<MoveType>& path);

}  // namespace fb::ai::common
