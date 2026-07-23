#include "core/wall_kick.hpp"

#include <array>

namespace fb::core {

namespace {

int GetKickIndex(int from, int to) {
    if (from == 0 && to == 1) return 0;
    if (from == 1 && to == 0) return 1;
    if (from == 1 && to == 2) return 2;
    if (from == 2 && to == 1) return 3;
    if (from == 2 && to == 3) return 4;
    if (from == 3 && to == 2) return 5;
    if (from == 3 && to == 0) return 6;
    if (from == 0 && to == 3) return 7;
    return 0;
}

const std::array<std::vector<Vec2i>, 8>& StandardKickData() {
    static const std::array<std::vector<Vec2i>, 8> kickData = {{
        {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
        {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
        {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
        {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
        {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
        {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
    }};
    return kickData;
}

const std::array<std::vector<Vec2i>, 8>& IPieceKickData() {
    static const std::array<std::vector<Vec2i>, 8> kickData = {{
        {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
        {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
        {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},
        {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},
        {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
        {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
        {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},
        {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},
    }};
    return kickData;
}

const std::array<std::vector<Vec2i>, 8>& LargeKickData() {
    static const std::array<std::vector<Vec2i>, 8> kickData = {{
        // 0->1
        {{0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {-3, 0}, {3, 0}, {-1, 1}, {-2, 1}, {-3, 1}, {0, -2}, {-1, -2}, {-2, -2}},
        // 1->0
        {{0, 0}, {1, 0}, {-1, 0}, {2, 0}, {-2, 0}, {3, 0}, {-3, 0}, {1, -1}, {2, -1}, {3, -1}, {0, 2}, {1, 2}, {2, 2}},
        // 1->2
        {{0, 0}, {1, 0}, {-1, 0}, {2, 0}, {-2, 0}, {3, 0}, {-3, 0}, {1, -1}, {2, -1}, {3, -1}, {0, 2}, {1, 2}, {2, 2}},
        // 2->1
        {{0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {-3, 0}, {3, 0}, {-1, 1}, {-2, 1}, {-3, 1}, {0, -2}, {-1, -2}, {-2, -2}},
        // 2->3
        {{0, 0}, {1, 0}, {-1, 0}, {2, 0}, {-2, 0}, {3, 0}, {-3, 0}, {1, 1}, {2, 1}, {3, 1}, {0, -2}, {1, -2}, {2, -2}},
        // 3->2
        {{0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {-3, 0}, {3, 0}, {-1, -1}, {-2, -1}, {-3, -1}, {0, 2}, {-1, 2}, {-2, 2}},
        // 3->0
        {{0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {-3, 0}, {3, 0}, {-1, -1}, {-2, -1}, {-3, -1}, {0, 2}, {-1, 2}, {-2, 2}},
        // 0->3
        {{0, 0}, {1, 0}, {-1, 0}, {2, 0}, {-2, 0}, {3, 0}, {-3, 0}, {1, 1}, {2, 1}, {3, 1}, {0, -2}, {1, -2}, {2, -2}},
    }};
    return kickData;
}

}  // namespace

KickTableType SelectKickTable(const std::string& minoName, int gridSize) {
    if (gridSize >= 5) {
        return KickTableType::Large;
    }
    if (minoName == "I" || gridSize == 4) {
        return KickTableType::IPiece;
    }
    return KickTableType::Standard;
}

const std::vector<Vec2i>& GetWallKicks(KickTableType type, int fromRotation, int toRotation) {
    int index = GetKickIndex(fromRotation, toRotation);
    switch (type) {
        case KickTableType::IPiece:
            return IPieceKickData()[index];
        case KickTableType::Large:
            return LargeKickData()[index];
        case KickTableType::Standard:
        default:
            return StandardKickData()[index];
    }
}

}  // namespace fb::core
