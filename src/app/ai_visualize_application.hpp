#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ai/player.hpp"
#include "app/renderer.hpp"
#include "core/game.hpp"

#include "app/key_config.hpp"

namespace fb::app {

// AIの自動プレイを眺めるための専用モード。人間の操作を一切想定していない。
//
// AiPlayer::Plan()で得た操作列(回転・移動・ハードドロップ)を、1操作ずつ
// moveDelay秒おきに再生する(1ピースを置くまでの過程が見えるようにするため)。
// このため core::Game::Tick()(自然落下・ロック遅延)を意図的に呼ばない。
// これは「AIが操作している間はTickを止める」という一般ルールではなく、
// このクラス専用の割り切りである。将来、人間 vs AIの対戦のように公平性が
// 必要なモードを作る場合、このクラスを流用してはならない――対戦では両者に
// 対してTick()を無条件で呼び続ける必要がある。
class AiVisualizeApplication {
public:
    // minosJsonPath: 読み込むミノ定義ファイル。
    // aiPlayer: 自動プレイに使うAI。
    // moveDelay: 1操作ごとに置く間隔(秒)。目で追えるようにするための演出用。
    // seed: 指定するとミノの出現列が再現可能になる。Restartキーでも同じシードを使い直す
    AiVisualizeApplication(const std::string& minosJsonPath, std::unique_ptr<ai::AiPlayer> aiPlayer,
                           float moveDelay = 0.3f, std::optional<std::uint64_t> seed = std::nullopt);

    int Run();

private:
    void ExecuteMove(ai::PlayerMove move);
    std::uint64_t NextSeed() const;

    core::Game game_;
    std::unique_ptr<ai::AiPlayer> ai_;
    Renderer renderer_;

    float moveDelay_;
    float cooldownTimer_ = 0.f;
    std::vector<ai::PlayerMove> pendingMoves_;  // 現在のピースについて、まだ再生していない操作列
    std::optional<std::uint64_t> fixedSeed_;

    bool prevRestartPressed_ = false;  // Restartキーのエッジ検出用
};

}  // namespace fb::app
