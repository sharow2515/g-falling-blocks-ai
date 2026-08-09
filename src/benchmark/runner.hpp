#pragma once

#include <cstdint>
#include <vector>

#include "ai/player.hpp"
#include "core/mino.hpp"

namespace fb::benchmark {

struct GameResult {
    int score = 0;
    int totalLines = 0;
    bool hitLineLimit = false;  // trueの場合、maxLinesに到達したため打ち切られたことを表す(ゲームオーバーではない)
    int totalPieces = 0;
    double decisionTimeMs = 0.0;  // Plan()呼び出しのみの合計時間
};

GameResult RunOneGame(ai::AiPlayer& player, const std::vector<core::Mino>& minos, std::uint64_t seed,
                      int maxLines);

}  // namespace fb::benchmark
