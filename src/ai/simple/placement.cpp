#include "ai/simple/placement.hpp"

#include <optional>
#include <utility>
#include <vector>

#include "core/wall_kick.hpp"

namespace fb::ai {

namespace {

struct PieceState {
    int rotation;
    core::Vec2i position;
};

// 試す回転の操作列。
const std::vector<PlayerMove> kRotationSequences[] = {
    {},
    {PlayerMove::RotateCW},
    {PlayerMove::RotateCCW},
    {PlayerMove::RotateCW, PlayerMove::RotateCW},
};

// rotate(RotateCW / RotateCCW)で1回転させる。キックを順に試し、置ける最初の位置を採用する。
std::optional<PieceState> TryRotate(const core::Board& board, const core::Mino& mino, PieceState state,
                                    PlayerMove rotate) {
    int direction = rotate == PlayerMove::RotateCW ? 1 : -1;
    int newRotation = ((state.rotation + direction) % 4 + 4) % 4;
    const std::vector<core::Vec2i>& newCells = mino.GetCells(newRotation);

    core::KickTableType kickType = core::SelectKickTable(mino.name, mino.gridSize);
    for (const auto& kick : core::GetWallKicks(kickType, state.rotation, newRotation)) {
        core::Vec2i newPos = state.position + kick;
        if (board.IsValidPosition(newCells, newPos)) return PieceState{newRotation, newPos};
    }
    return std::nullopt;
}

// rotationsを順に当てはめる
std::optional<PieceState> ApplyRotations(const core::Board& board, const core::Mino& mino, PieceState state,
                                         const std::vector<PlayerMove>& rotations) {
    for (PlayerMove rotate : rotations) {
        std::optional<PieceState> next = TryRotate(board, mino, state, rotate);
        if (!next) return std::nullopt;
        state = *next;
    }
    return state;
}

core::Vec2i DropToBottom(const core::Board& board, const std::vector<core::Vec2i>& cells, core::Vec2i position) {
    while (board.IsValidPosition(cells, position + core::Vec2i::Down())) {
        position = position + core::Vec2i::Down();
    }
    return position;
}

}  // namespace

std::vector<Placement> EnumerateReachablePlacements(const core::Board& board, const core::Mino& mino,
                                                    int spawnRotation, core::Vec2i spawnPosition) {
    std::vector<Placement> placements;

    for (const std::vector<PlayerMove>& rotations : kRotationSequences) {
        std::optional<PieceState> rotated = ApplyRotations(board, mino, {spawnRotation, spawnPosition}, rotations);
        if (!rotated) continue;

        const std::vector<core::Vec2i>& cells = mino.GetCells(rotated->rotation);

        // pos(回転後に、左右移動した位置)から落として、候補に加える。
        auto addPlacement = [&](core::Vec2i pos, std::vector<PlayerMove> moves) {
            core::Vec2i landed = DropToBottom(board, cells, pos);
            Placement placement;
            placement.cells.reserve(cells.size());
            for (const auto& c : cells) placement.cells.push_back(c + landed);
            placement.moves = std::move(moves);
            placement.moves.push_back(PlayerMove::HardDrop);
            placements.push_back(std::move(placement));
        };

        // 左右移動なし
        addPlacement(rotated->position, rotations);

        // 左右移動
        for (PlayerMove slide : {PlayerMove::Left, PlayerMove::Right}) {
            core::Vec2i step = slide == PlayerMove::Left ? core::Vec2i::Left() : core::Vec2i::Right();
            std::vector<PlayerMove> moves = rotations;
            core::Vec2i pos = rotated->position;
            while (board.IsValidPosition(cells, pos + step)) {
                pos = pos + step;
                moves.push_back(slide);
                addPlacement(pos, moves);
            }
        }
    }

    return placements;
}

}  // namespace fb::ai
