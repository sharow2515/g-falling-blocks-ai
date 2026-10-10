#pragma once

#include <string>
#include <vector>

#include "core/game.hpp"

namespace fb::ai {

// AIプレイヤーが1手の意思決定として発行する、最小単位の操作。
// Plan()から実行順に並んだ操作列として返される。
enum class PlayerMove {
    Left,
    Right,
    Down,
    RotateCW,
    RotateCCW,
    Hold,
    HardDrop,
};

// 全AIプレイヤーが実装する共通インターフェース。
class AiPlayer {
public:
    virtual ~AiPlayer() = default;


    // 操作できない状態(ゲームオーバーなど)や、おける場所がない場合は空を返す。
    virtual std::vector<PlayerMove> Plan(core::Game& game) = 0;

    // ログ・ベンチマーク結果の表示などに使う名前。
    virtual std::string Name() const = 0;
};

}  // namespace fb::ai
