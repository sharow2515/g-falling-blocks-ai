#pragma once

#include <string>
#include <vector>

#include "core/types.hpp"

namespace fb::core {

enum class KickTableType {
    Standard,  // O以外の3x3ミノ等
    IPiece,    // "I" という名前のミノ、または gridSize==4
    Large,     // gridSize >= 5
};

KickTableType SelectKickTable(const std::string& minoName, int gridSize);

// fromRotation -> toRotation (どちらも0-3、隣接遷移のみ) のキックオフセット候補を、
// 試す順番のまま返す。位置0は必ず(0,0)。
const std::vector<Vec2i>& GetWallKicks(KickTableType type, int fromRotation, int toRotation);

}  // namespace fb::core
