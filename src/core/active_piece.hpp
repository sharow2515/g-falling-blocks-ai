#pragma once

#include <vector>

#include "core/board.hpp"
#include "core/mino.hpp"
#include "core/types.hpp"
#include "core/wall_kick.hpp"

namespace fb::core {

struct LockResult {
    bool locked = false;
    int linesCleared = 0;
    bool isTSpin = false;
    std::vector<int> clearedRows;
};

struct TickResult {
    int scoreDelta = 0;  // ソフトドロップ分の加点
    LockResult lockResult;
};

struct PieceConfig {
    float softDropMultiplier = 20.f;
    int maxLockResets = 15;
    float lockDelay = 0.5f;
};

class ActivePiece {
public:
    bool Initialize(const Mino& mino, Board& board, float fallInterval, const PieceConfig& config = {});

    bool MoveLeft(Board& board);
    bool MoveRight(Board& board);
    bool MoveDown(Board& board);

    // direction: +1 = 時計回り, -1 = 反時計回り
    void Rotate(Board& board, int direction);

    void SetSoftDropping(bool active) { softDropping_ = active; }

    TickResult Tick(Board& board, float dt);

    LockResult HardDrop(Board& board, int& dropDistance);

    void Deactivate() { isActive_ = false; }

    [[nodiscard]] bool IsActive() const { return isActive_; }
    [[nodiscard]] Vec2i Position() const { return position_; }
    [[nodiscard]] int Rotation() const { return rotation_; }
    [[nodiscard]] const std::vector<Vec2i>& Cells() const { return currentCells_; }
    [[nodiscard]] const Mino* MinoPtr() const { return mino_; }

    [[nodiscard]] std::vector<Vec2i> WorldCells() const;
    [[nodiscard]] std::vector<Vec2i> GhostWorldCells(const Board& board) const;

private:
    bool MoveInternal(Board& board, Vec2i direction);
    void ResetLockDelay();
    LockResult LockPiece(Board& board);

    const Mino* mino_ = nullptr;
    Vec2i position_;
    int rotation_ = 0;
    std::vector<Vec2i> currentCells_;

    float fallTimer_ = 0.f;
    float fallInterval_ = 1.f;

    float lockTimer_ = 0.f;
    bool isLocking_ = false;
    int lockResetCount_ = 0;
    int lowestY_ = 0;

    bool softDropping_ = false;
    bool isActive_ = false;

    bool lastMoveWasRotation_ = false;
    int lastKickIndex_ = 0;

    PieceConfig config_;
};

}  // namespace fb::core
