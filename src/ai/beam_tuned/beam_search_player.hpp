#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "ai/beam_tuned/evaluator.hpp"
#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai::beam_tuned {

// searchDepthの上限。Renderer::DrawNextが表示するNext情報は5個まで
inline constexpr int kMaxSearchDepth = 5;

class BeamSearchPlayer : public AiPlayer {
public:
    BeamSearchPlayer(int beamWidth, int searchDepth, Weights weights = Weights{});

    void Update(core::Game& game) override;
    std::vector<PlayerMove> Plan(core::Game& game) override;
    std::string Name() const override { return "BeamTuned"; }

private:
    std::vector<PlayerMove> BuildPlan(core::Game& game);

    int beamWidth_;
    int searchDepth_;
    Weights weights_;
    int lastSeenSpawnEventId_ = -1;
};

}  // namespace fb::ai::beam_tuned
