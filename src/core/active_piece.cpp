#include "core/active_piece.hpp"

#include <limits>

namespace fb::core {

bool ActivePiece::Initialize(const Mino& mino, Board& board, float fallInterval, const PieceConfig& config) {
    mino_ = &mino;
    config_ = config;
    rotation_ = 0;
    fallInterval_ = fallInterval;
    fallTimer_ = 0.f;
    lockTimer_ = 0.f;
    isLocking_ = false;
    lockResetCount_ = 0;
    softDropping_ = false;
    isActive_ = true;

    currentCells_ = mino_->GetCells(rotation_);

    int maxCellY = std::numeric_limits<int>::min();
    for (const auto& cell : currentCells_) {
        if (cell.y > maxCellY) maxCellY = cell.y;
    }

    int spawnX = board.Width() / 2 - mino_->gridSize / 2;
    int spawnY = board.Height() - maxCellY - 1;
    position_ = Vec2i{spawnX, spawnY};

    // 初期位置で有効かチェック
    if (!board.IsValidPosition(currentCells_, position_)) {
        position_.y += 1;
        if (!board.IsValidPosition(currentCells_, position_)) {
            position_.y += 1;
            if (!board.IsValidPosition(currentCells_, position_)) {
                isActive_ = false;
                return false;
            }
        }
    }

    lowestY_ = position_.y;
    lastMoveWasRotation_ = false;
    lastKickIndex_ = 0;
    return true;
}

bool ActivePiece::MoveInternal(Board& board, Vec2i direction) {
    Vec2i newPos = position_ + direction;

    if (!board.IsValidPosition(currentCells_, newPos)) {
        return false;
    }

    position_ = newPos;
    lastMoveWasRotation_ = false;

    if (direction == Vec2i::Down() && position_.y < lowestY_) {
        lowestY_ = position_.y;
    }

    if (isLocking_ && direction != Vec2i::Down()) {
        ResetLockDelay();
    }

    return true;
}

bool ActivePiece::MoveLeft(Board& board) { return MoveInternal(board, Vec2i::Left()); }
bool ActivePiece::MoveRight(Board& board) { return MoveInternal(board, Vec2i::Right()); }
bool ActivePiece::MoveDown(Board& board) { return MoveInternal(board, Vec2i::Down()); }

void ActivePiece::Rotate(Board& board, int direction) {
    int newRotation = ((rotation_ + direction) % 4 + 4) % 4;
    std::vector<Vec2i> newCells = mino_->GetCells(newRotation);

    KickTableType type = SelectKickTable(mino_->name, mino_->gridSize);
    std::vector<Vec2i> kicks = GetWallKicks(type, rotation_, newRotation);

    for (std::size_t i = 0; i < kicks.size(); ++i) {
        Vec2i newPos = position_ + kicks[i];
        if (board.IsValidPosition(newCells, newPos)) {
            position_ = newPos;
            rotation_ = newRotation;
            currentCells_ = newCells;
            lastMoveWasRotation_ = true;
            lastKickIndex_ = static_cast<int>(i);

            if (isLocking_) {
                ResetLockDelay();
            }
            return;
        }
    }
}

TickResult ActivePiece::Tick(Board& board, float dt) {
    TickResult result;
    if (!isActive_) return result;

    float currentInterval = softDropping_ ? fallInterval_ / config_.softDropMultiplier : fallInterval_;

    fallTimer_ += dt;
    if (fallTimer_ >= currentInterval) {
        fallTimer_ = 0.f;
        if (MoveInternal(board, Vec2i::Down())) {
            if (softDropping_) {
                result.scoreDelta += 1;
            }
        } else {
            if (!isLocking_) {
                isLocking_ = true;
                lockTimer_ = 0.f;
            }
        }
    }

    if (isLocking_) {
        if (board.IsValidPosition(currentCells_, position_ + Vec2i::Down())) {
            isLocking_ = false;
            lockTimer_ = 0.f;
        } else {
            lockTimer_ += dt;
            if (lockTimer_ >= config_.lockDelay) {
                result.lockResult = LockPiece(board);
            }
        }
    }

    return result;
}

LockResult ActivePiece::HardDrop(Board& board, int& dropDistance) {
    dropDistance = 0;
    while (board.IsValidPosition(currentCells_, position_ + Vec2i::Down())) {
        position_ = position_ + Vec2i::Down();
        ++dropDistance;
    }
    return LockPiece(board);
}

void ActivePiece::ResetLockDelay() {
    if (lockResetCount_ < config_.maxLockResets) {
        lockTimer_ = 0.f;
        ++lockResetCount_;
    }
}

LockResult ActivePiece::LockPiece(Board& board) {
    isActive_ = false;

    bool isTSpin = false;
    if (mino_->name == "T" && lastMoveWasRotation_) {
        Vec2i center = position_ + Vec2i{1, 1};
        int corners = board.CountTSpinCorners(center);
        isTSpin = corners >= 3;
    }

    board.PlacePiece(currentCells_, position_, mino_->color);

    LockResult result;
    result.locked = true;
    result.linesCleared = board.ClearFullLines(result.clearedRows);
    result.isTSpin = isTSpin;
    return result;
}

std::vector<Vec2i> ActivePiece::WorldCells() const {
    std::vector<Vec2i> cells;
    if (!isActive_) return cells;
    cells.reserve(currentCells_.size());
    for (const auto& c : currentCells_) cells.push_back(c + position_);
    return cells;
}

std::vector<Vec2i> ActivePiece::GhostWorldCells(const Board& board) const {
    std::vector<Vec2i> cells;
    if (!isActive_) return cells;
    int ghostY = board.GetGhostY(currentCells_, position_);
    Vec2i ghostPos{position_.x, ghostY};
    cells.reserve(currentCells_.size());
    for (const auto& c : currentCells_) cells.push_back(c + ghostPos);
    return cells;
}

}  // namespace fb::core
