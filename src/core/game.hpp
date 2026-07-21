#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "core/active_piece.hpp"
#include "core/bag_generator.hpp"
#include "core/board.hpp"
#include "core/mino.hpp"

namespace fb::core {

struct GameConfig {
    int previewCount = 5;
    float initialFallInterval = 1.0f;
    float minFallInterval = 0.05f;
    PieceConfig pieceConfig{};
};

// 全体フロー・bag生成・スコア/レベル・ホールドを管理する。
class Game {
public:
    Game(std::vector<Mino> allMinos, GameConfig config = {});

    void StartGame(std::uint64_t seed);
    void RestartGame(std::uint64_t seed);

    // 自然落下・ロック遅延の経過を進める(一時停止中・ゲームオーバー中は何もしない)。
    void Tick(float dt);

    // コマンド
    void MoveLeft();
    void MoveRight();
    bool MoveDown();

    void SetSoftDropping(bool active);
    void HardDrop();
    void RotateCW();
    void RotateCCW();
    void Hold();

    void TogglePause();

    // 参照用ゲッター
    [[nodiscard]] const Board& GetBoard() const { return board_; }
    [[nodiscard]] const ActivePiece& GetActivePiece() const { return activePiece_; }
    [[nodiscard]] std::vector<const Mino*> PeekNext(int count) const;
    [[nodiscard]] const Mino* GetHoldMino() const { return holdMino_; }
    [[nodiscard]] int GetScore() const { return score_; }
    [[nodiscard]] int GetLevel() const { return level_; }
    [[nodiscard]] int GetTotalLines() const { return totalLines_; }
    [[nodiscard]] bool IsGameOver() const { return isGameOver_; }
    [[nodiscard]] bool IsPaused() const { return isPaused_; }
    [[nodiscard]] const std::vector<int>& LastClearedRows() const { return lastClearedRows_; }

    // ライン消去が発生するたびに1増える。
    // app層(Renderer)が「新しい消去イベントか」を検知するためのもの。
    [[nodiscard]] int ClearEventId() const { return clearEventId_; }

    // 新しいピースが出現するたびに1増える。app層(InputController)が「ピースが
    // 切り替わったか」を検知してDAS/ARR状態をリセットするためのもの
    [[nodiscard]] int SpawnEventId() const { return spawnEventId_; }

private:
    [[nodiscard]] bool CanAcceptInput() const;
    void FillQueue();
    void SpawnNextPiece();
    void HandleLockResult(const LockResult& result);
    void AddTSpinScore(int lines);
    void AddLineScore(int lines);
    void UpdateFallSpeed();
    void TriggerGameOver();

    GameConfig config_;
    Board board_;
    BagGenerator bagGenerator_;
    std::deque<const Mino*> minoQueue_;
    ActivePiece activePiece_;

    const Mino* holdMino_ = nullptr;
    bool canHold_ = true;

    int score_ = 0;
    int level_ = 1;
    int totalLines_ = 0;
    bool isGameOver_ = false;
    bool isPaused_ = false;

    float currentFallInterval_ = 1.0f;

    std::vector<int> lastClearedRows_;
    int clearEventId_ = 0;
    int spawnEventId_ = 0;
};

}  // namespace fb::core
