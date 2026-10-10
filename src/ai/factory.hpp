#pragma once

#include <memory>
#include <optional>
#include <string>

#include "ai/player.hpp"

namespace fb::ai {

struct AiOptions {
    std::string name = "simple";
    int beamWidth = 20;                  // beam系のみ
    int beamDepth = 2;                   // beam系のみ(0〜5)
    std::optional<std::string> weights;  // beam系のみ。未指定ならデフォルトの重み
};

std::unique_ptr<AiPlayer> MakeAiPlayer(const AiOptions& options);

}  // namespace fb::ai
