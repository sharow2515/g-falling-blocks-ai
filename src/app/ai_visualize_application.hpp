#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ai/player.hpp"
#include "app/application_base.hpp"

namespace fb::app {

// AIのプレイを見るためのモード
// AiPlayer::Plan()で得た操作列(回転・移動・ハードドロップ)を、1操作ずつ
// moveDelay秒おきに再生する(1ピースを置くまでの過程が見えるようにするため)。
// このため core::Game::Tick()(自然落下・ロック遅延)を呼ばない。
class AiVisualizeApplication : public ApplicationBase {
public:
    AiVisualizeApplication(const std::string& minosJsonPath, std::unique_ptr<ai::AiPlayer> aiPlayer,
                           float moveDelay = 0.3f, std::optional<std::uint64_t> seed = std::nullopt);

private:
    void Update(float deltaSeconds) override;

    std::unique_ptr<ai::AiPlayer> ai_;

    float moveDelay_;
    float cooldownTimer_ = 0.f;
    std::vector<ai::PlayerMove> pendingMoves_;  // 現在のピースについて、まだ再生していない操作列
};

}  // namespace fb::app
