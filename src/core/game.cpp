#include "core/game.hpp"

#include <algorithm>
#include <cmath>

namespace fb::core {

Game::Game(std::vector<Mino> allMinos, GameConfig config)
    : config_(config), board_(), bagGenerator_(std::move(allMinos), 0) {}

void Game::StartGame(std::uint64_t seed) {
    score_ = 0;
    level_ = 1;
    totalLines_ = 0;
    isGameOver_ = false;
    isPaused_ = false;
    holdMino_ = nullptr;
    canHold_ = true;
    currentFallInterval_ = config_.initialFallInterval;

    board_.ClearBoard();
    minoQueue_.clear();
    lastClearedRows_.clear();
    bagGenerator_.Reseed(seed);

    FillQueue();
    SpawnNextPiece();
}

void Game::RestartGame(std::uint64_t seed) { StartGame(seed); }

bool Game::CanAcceptInput() const {
    return !isGameOver_ && !isPaused_ && activePiece_.IsActive();
}

void Game::Tick(float dt) {
    if (isGameOver_ || isPaused_) return;
    if (!activePiece_.IsActive()) return;

    TickResult tr = activePiece_.Tick(board_, dt);
    score_ += tr.scoreDelta;

    if (tr.lockResult.locked) {
        HandleLockResult(tr.lockResult);
    }
}

void Game::MoveLeft() {
    if (CanAcceptInput()) activePiece_.MoveLeft(board_);
}

void Game::MoveRight() {
    if (CanAcceptInput()) activePiece_.MoveRight(board_);
}

bool Game::MoveDown() {
    if (!CanAcceptInput()) return false;
    return activePiece_.MoveDown(board_);
}

void Game::SetSoftDropping(bool active) {
    if (CanAcceptInput()) activePiece_.SetSoftDropping(active);
}

void Game::RotateCW() {
    if (CanAcceptInput()) activePiece_.Rotate(board_, 1);
}

void Game::RotateCCW() {
    if (CanAcceptInput()) activePiece_.Rotate(board_, -1);
}

void Game::HardDrop() {
    if (!CanAcceptInput()) return;

    int dropDistance = 0;
    LockResult result = activePiece_.HardDrop(board_, dropDistance);
    score_ += dropDistance * 2;
    HandleLockResult(result);
}

void Game::Hold() {
    if (!canHold_ || isGameOver_ || isPaused_) return;

    canHold_ = false;
    const Mino* currentMino = activePiece_.MinoPtr();
    activePiece_.Deactivate();

    if (holdMino_ != nullptr) {
        const Mino* temp = holdMino_;
        holdMino_ = currentMino;
        if (!activePiece_.Initialize(*temp, board_, currentFallInterval_, config_.pieceConfig)) {
            TriggerGameOver();
        } else {
            ++spawnEventId_;
        }
    } else {
        holdMino_ = currentMino;
        SpawnNextPiece();
    }
}

void Game::TogglePause() { isPaused_ = !isPaused_; }

void Game::FillQueue() {
    while (minoQueue_.size() <= static_cast<std::size_t>(config_.previewCount) + 1) {
        auto bag = bagGenerator_.GenerateBag();
        for (const Mino* m : bag) {
            minoQueue_.push_back(m);
        }
    }
}

void Game::SpawnNextPiece() {
    if (isGameOver_) return;

    FillQueue();
    const Mino* nextMino = minoQueue_.front();
    minoQueue_.pop_front();
    canHold_ = true;

    if (!activePiece_.Initialize(*nextMino, board_, currentFallInterval_, config_.pieceConfig)) {
        TriggerGameOver();
    } else {
        ++spawnEventId_;
    }
}

void Game::HandleLockResult(const LockResult& result) {
    lastClearedRows_ = result.clearedRows;

    if (result.linesCleared > 0) {
        ++clearEventId_;

        if (result.isTSpin) {
            AddTSpinScore(result.linesCleared);
        } else {
            AddLineScore(result.linesCleared);
        }
        totalLines_ += result.linesCleared;

        int newLevel = totalLines_ / 10 + 1;
        if (newLevel > level_) {
            level_ = newLevel;
            UpdateFallSpeed();
        }
    } else if (result.isTSpin) {
        // ライン消去なしのTスピン (T-Spin Zero)
        score_ += 400 * level_;
    }

    if (board_.IsGameOver()) {
        TriggerGameOver();
        return;
    }

    SpawnNextPiece();
}

void Game::AddTSpinScore(int lines) {
    int baseScore;
    switch (lines) {
        case 1: baseScore = 800; break;   // T-Spin Single
        case 2: baseScore = 1200; break;  // T-Spin Double
        case 3: baseScore = 1600; break;  // T-Spin Triple
        default: baseScore = lines * 400; break;
    }
    score_ += baseScore * level_;
}

void Game::AddLineScore(int lines) {
    int baseScore;
    switch (lines) {
        case 1: baseScore = 100; break;
        case 2: baseScore = 300; break;
        case 3: baseScore = 500; break;
        case 4: baseScore = 800; break;
        default: baseScore = lines * 200; break;
    }
    score_ += baseScore * level_;
}

void Game::UpdateFallSpeed() {
    currentFallInterval_ = std::max(
        config_.minFallInterval,
        config_.initialFallInterval *
            std::pow(0.8f - static_cast<float>(level_ - 1) * 0.007f, static_cast<float>(level_ - 1)));
}

void Game::TriggerGameOver() {
    isGameOver_ = true;
    activePiece_.Deactivate();
}

std::vector<const Mino*> Game::PeekNext(int count) const {
    std::vector<const Mino*> result;
    int n = std::min<int>(count, static_cast<int>(minoQueue_.size()));
    result.reserve(n);
    auto it = minoQueue_.begin();
    for (int i = 0; i < n; ++i, ++it) {
        result.push_back(*it);
    }
    return result;
}

}  // namespace fb::core
