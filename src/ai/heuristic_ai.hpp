#pragma once

#include "core/game.hpp"

namespace fb::ai {

// 評価関数ベースの1手読みAI。Holdは使わない。
class HeuristicAi {
public:
    // 呼び出しごとに1回チェックし、新しいピースが出現していれば
    // (Game::SpawnEventId()の変化で検知)最善手を1回だけ決定・実行する。
    void Update(core::Game& game);

private:
    void PlayBestMove(core::Game& game);

    int lastSeenSpawnEventId_ = -1;
};

}  // namespace fb::ai
