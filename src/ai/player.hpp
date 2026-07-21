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

    // 呼び出しごとに1回チェックし、必要なら core::Game にコマンドを発行する
    // (判断から実行まで即座に完結させる、通常プレイ・ベンチマーク向けの入口)。
    // 実装は通常 Game::SpawnEventId() 等の変化を見て、新しいピースが出現した
    // タイミングでのみ手を打つ(毎フレーム呼ばれる前提の設計)。
    virtual void Update(core::Game& game) = 0;

    // Update()と同じ判断ロジックで最善手を決めるが、Gameへは何も発行せず
    // 操作列だけを返す。呼び出し側が1操作ずつ間隔を空けて再生したい場合
    // (AiVisualizeApplication等)に使う。
    // 新しいピースが出現していない・置く場所がない場合は空のvectorを返す。
    virtual std::vector<PlayerMove> Plan(core::Game& game) = 0;

    // ログ・ベンチマーク結果の表示などに使う名前。
    virtual std::string Name() const = 0;
};

}  // namespace fb::ai
