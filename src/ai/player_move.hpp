#pragma once

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

}  // namespace fb::ai
