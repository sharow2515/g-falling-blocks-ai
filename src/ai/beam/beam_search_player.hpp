#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "ai/beam/evaluator.hpp"
#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai::beam {

// searchDepthの上限。Renderer::DrawNextが表示するNext情報は5個まで
inline constexpr int kMaxSearchDepth = 5;

class BeamSearchPlayer : public AiPlayer {
public:
    BeamSearchPlayer(int beamWidth, int searchDepth, Weights weights = Weights{});

    std::vector<PlayerMove> Plan(const core::Game& game) override;
    std::string Name() const override { return "Beam"; }

private:
    int beamWidth_;
    int searchDepth_;
    Weights weights_;
};

}  // namespace fb::ai::beam
