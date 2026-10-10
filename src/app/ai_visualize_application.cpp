#include "app/ai_visualize_application.hpp"

#include "ai/common/move_execution.hpp"

namespace fb::app {

AiVisualizeApplication::AiVisualizeApplication(const std::string& minosJsonPath,
                                              std::unique_ptr<ai::AiPlayer> aiPlayer, float moveDelay,
                                              std::optional<std::uint64_t> seed)
    : ApplicationBase(minosJsonPath, seed, "FallingBlocks - AI Visualize"),
      ai_(std::move(aiPlayer)),
      moveDelay_(moveDelay) {}

void AiVisualizeApplication::Update(float deltaSeconds) {
    // 一時停止とリスタートだけ処理する。
    input_.Update(game_, deltaSeconds, /*controlPiece=*/false);

    if (TryRestart()) {
        pendingMoves_.clear();
        cooldownTimer_ = 0.f;
        return;
    }
    if (game_.IsGameOver() || game_.IsPaused()) return;

    cooldownTimer_ -= deltaSeconds;
    if (cooldownTimer_ > 0.f) return;

    if (pendingMoves_.empty()) {
        pendingMoves_ = ai_->Plan(game_);
    }
    if (!pendingMoves_.empty()) {
        ai::PlayerMove move = pendingMoves_.front();
        pendingMoves_.erase(pendingMoves_.begin());
        ai::common::ExecuteMove(game_, move);
    }
    cooldownTimer_ = moveDelay_;
}

}  // namespace fb::app
