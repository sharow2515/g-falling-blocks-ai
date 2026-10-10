#pragma once

#include <string>
#include <vector>

#include "ai/player_move.hpp"
#include "core/game.hpp"

namespace fb::ai {

// 全AIプレイヤーが実装する共通インターフェース。
class AiPlayer {
public:
    virtual ~AiPlayer() = default;


    // 操作できない状態(ゲームオーバーなど)や、おける場所がない場合は空を返す。
    virtual std::vector<PlayerMove> Plan(const core::Game& game) = 0;

    // ログ・ベンチマーク結果の表示などに使う名前。
    virtual std::string Name() const = 0;
};

}  // namespace fb::ai
