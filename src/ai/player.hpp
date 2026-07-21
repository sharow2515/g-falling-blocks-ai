#pragma once

#include <string>

#include "core/game.hpp"

namespace fb::ai {

// 全AIプレイヤーが実装する共通インターフェース。
class AiPlayer {
public:
    virtual ~AiPlayer() = default;

    // 呼び出しごとに1回チェックし、必要なら core::Game にコマンドを発行する。
    // 実装は通常 Game::SpawnEventId() 等の変化を見て、新しいピースが出現した
    // タイミングでのみ手を打つ(毎フレーム呼ばれる前提の設計)。
    virtual void Update(core::Game& game) = 0;

    // ログ・ベンチマーク結果の表示などに使う名前。
    virtual std::string Name() const = 0;
};

}  // namespace fb::ai
