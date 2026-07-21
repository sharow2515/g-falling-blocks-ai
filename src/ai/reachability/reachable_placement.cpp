#include "ai/reachability/reachable_placement.hpp"

#include <array>
#include <map>
#include <queue>
#include <utility>

#include "core/wall_kick.hpp"

namespace fb::ai::reachability {

namespace {

struct StateKey {
    int rotation;
    int x;
    int y;

    bool operator<(const StateKey& other) const {
        if (rotation != other.rotation) return rotation < other.rotation;
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

}  // namespace

std::vector<ReachablePlacement> SearchReachablePlacements(const core::Board& board, const core::Mino& mino,
                                                            int spawnRotation, core::Vec2i spawnPosition) {
    std::vector<ReachablePlacement> results;

    // 回転ごとのセル形状はミノ・回転の組み合わせで固定なので先に計算しておく。
    std::array<std::vector<core::Vec2i>, 4> cellsByRotation;
    for (int r = 0; r < 4; ++r) cellsByRotation[r] = mino.GetCells(r);

    core::KickTableType kickType = core::SelectKickTable(mino.name, mino.gridSize);

    StateKey startKey{spawnRotation, spawnPosition.x, spawnPosition.y};
    std::map<StateKey, std::vector<MoveType>> visited;
    visited[startKey] = {};

    std::queue<StateKey> pending;
    pending.push(startKey);

    while (!pending.empty()) {
        StateKey current = pending.front();
        pending.pop();

        core::Vec2i pos{current.x, current.y};
        const std::vector<core::Vec2i>& cells = cellsByRotation[current.rotation];
        const std::vector<MoveType>& currentPath = visited.at(current);

        auto tryVisit = [&](core::Vec2i newPos, int newRotation, MoveType moveType) {
            StateKey newKey{newRotation, newPos.x, newPos.y};
            if (visited.find(newKey) != visited.end()) return;

            std::vector<MoveType> newPath = currentPath;
            newPath.push_back(moveType);
            visited.emplace(newKey, std::move(newPath));
            pending.push(newKey);
        };

        // 左右下への移動
        if (board.IsValidPosition(cells, pos + core::Vec2i::Left())) {
            tryVisit(pos + core::Vec2i::Left(), current.rotation, MoveType::Left);
        }
        if (board.IsValidPosition(cells, pos + core::Vec2i::Right())) {
            tryVisit(pos + core::Vec2i::Right(), current.rotation, MoveType::Right);
        }
        if (board.IsValidPosition(cells, pos + core::Vec2i::Down())) {
            tryVisit(pos + core::Vec2i::Down(), current.rotation, MoveType::Down);
        }

        // 回転(時計回り・反時計回り)。ActivePiece::Rotateと同じく、
        // キック候補を順番に試して最初に成立したものだけを採用する。
        for (int dir : {1, -1}) {
            int newRotation = ((current.rotation + dir) % 4 + 4) % 4;
            std::vector<core::Vec2i> kicks = core::GetWallKicks(kickType, current.rotation, newRotation);
            const std::vector<core::Vec2i>& newCells = cellsByRotation[newRotation];

            for (const auto& kick : kicks) {
                core::Vec2i newPos = pos + kick;
                if (board.IsValidPosition(newCells, newPos)) {
                    MoveType moveType = (dir == 1) ? MoveType::RotateCW : MoveType::RotateCCW;
                    tryVisit(newPos, newRotation, moveType);
                    break;
                }
            }
        }

        // これ以上下に動けなければ「置ける場所」の候補として記録する
        // 探索は打ち切らない
        if (!board.IsValidPosition(cells, pos + core::Vec2i::Down())) {
            ReachablePlacement placement;
            placement.rotation = current.rotation;
            placement.position = pos;
            placement.cells.reserve(cells.size());
            for (const auto& c : cells) placement.cells.push_back(c + pos);
            placement.path = currentPath;
            results.push_back(std::move(placement));
        }
    }

    return results;
}

}  // namespace fb::ai::reachability
