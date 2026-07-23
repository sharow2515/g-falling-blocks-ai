#include "ai/common/reachable_search.hpp"

#include <algorithm>
#include <queue>

#include "ai/common/occupancy_grid.hpp"
#include "core/wall_kick.hpp"

namespace fb::ai::common {

namespace {

struct State {
    int rotation;
    int x;
    int y;
};

class StateSpace {
public:
    StateSpace(const core::Board& board, int gridSize)
        : width_(board.Width() + 2 * gridSize),
          height_(board.TotalHeight() + 2 * gridSize),
          offsetX_(gridSize),
          offsetY_(gridSize) {}

    int Index(const State& s) const { return (s.rotation * width_ + (s.x + offsetX_)) * height_ + (s.y + offsetY_); }

    int Size() const { return 4 * width_ * height_; }

private:
    int width_;
    int height_;
    int offsetX_;
    int offsetY_;
};

}  // namespace

std::vector<ReachablePlacement> SearchReachablePlacements(const core::Board& board, const core::Mino& mino,
                                                            int spawnRotation, core::Vec2i spawnPosition) {
    std::vector<ReachablePlacement> results;

    // 探索中はboardが変化しないため、占有状態を1回だけビットマスク化して使い回す。
    OccupancyGrid grid(board);

    // 回転ごとのセル形状はMino::GetCells内でキャッシュされているため、
    // ここで改めて配列にコピーする必要はない。
    core::KickTableType kickType = core::SelectKickTable(mino.name, mino.gridSize);

    StateSpace space(board, mino.gridSize);
    std::vector<bool> visited(static_cast<std::size_t>(space.Size()), false);
    std::vector<int> parent(static_cast<std::size_t>(space.Size()), -1);
    std::vector<MoveType> moveFromParent(static_cast<std::size_t>(space.Size()));

    // 終端状態から親を辿って、そこまでの手順を復元する。
    auto reconstructPath = [&](int index) {
        std::vector<MoveType> path;
        while (parent[index] != -1) {
            path.push_back(moveFromParent[index]);
            index = parent[index];
        }
        std::reverse(path.begin(), path.end());
        return path;
    };

    State start{spawnRotation, spawnPosition.x, spawnPosition.y};
    int startIndex = space.Index(start);
    visited[startIndex] = true;

    std::queue<State> pending;
    pending.push(start);

    while (!pending.empty()) {
        State current = pending.front();
        pending.pop();

        core::Vec2i pos{current.x, current.y};
        const std::vector<core::Vec2i>& cells = mino.GetCells(current.rotation);
        int currentIndex = space.Index(current);

        auto tryVisit = [&](core::Vec2i newPos, int newRotation, MoveType moveType) {
            State next{newRotation, newPos.x, newPos.y};
            int nextIndex = space.Index(next);
            if (visited[nextIndex]) return;

            visited[nextIndex] = true;
            parent[nextIndex] = currentIndex;
            moveFromParent[nextIndex] = moveType;
            pending.push(next);
        };

        // 回転(時計回り・反時計回り)。ActivePiece::Rotateと同じく、
        // キック候補を順番に試して最初に成立したものだけを採用する。
        for (int dir : {1, -1}) {
            int newRotation = ((current.rotation + dir) % 4 + 4) % 4;
            const std::vector<core::Vec2i>& kicks = core::GetWallKicks(kickType, current.rotation, newRotation);
            const std::vector<core::Vec2i>& newCells = mino.GetCells(newRotation);

            for (const auto& kick : kicks) {
                core::Vec2i newPos = pos + kick;
                if (grid.IsValidPosition(newCells, newPos)) {
                    MoveType moveType = (dir == 1) ? MoveType::RotateCW : MoveType::RotateCCW;
                    tryVisit(newPos, newRotation, moveType);
                    break;
                }
            }
        }

        // 左右下への移動
        if (grid.IsValidPosition(cells, pos + core::Vec2i::Left())) {
            tryVisit(pos + core::Vec2i::Left(), current.rotation, MoveType::Left);
        }
        if (grid.IsValidPosition(cells, pos + core::Vec2i::Right())) {
            tryVisit(pos + core::Vec2i::Right(), current.rotation, MoveType::Right);
        }
        if (grid.IsValidPosition(cells, pos + core::Vec2i::Down())) {
            tryVisit(pos + core::Vec2i::Down(), current.rotation, MoveType::Down);
        }

        // これ以上下に動けなければ「置ける場所」の候補として記録する
        // 探索は打ち切らない
        if (!grid.IsValidPosition(cells, pos + core::Vec2i::Down())) {
            ReachablePlacement placement;
            placement.rotation = current.rotation;
            placement.position = pos;
            placement.cells.reserve(cells.size());
            for (const auto& c : cells) placement.cells.push_back(c + pos);
            placement.path = reconstructPath(currentIndex);
            results.push_back(std::move(placement));
        }
    }

    return results;
}

}  // namespace fb::ai::common
