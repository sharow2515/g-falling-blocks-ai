#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai::beam {

// searchDepthの上限。Renderer::DrawNextが表示するNext情報は5個まで
inline constexpr int kMaxSearchDepth = 5;

class BeamSearchPlayer : public AiPlayer {
public:
    BeamSearchPlayer(int beamWidth, int searchDepth);

    void Update(core::Game& game) override;
    std::vector<PlayerMove> Plan(core::Game& game) override;
    std::string Name() const override { return "Beam"; }

private:
    std::vector<PlayerMove> BuildPlan(core::Game& game);

    int beamWidth_;
    int searchDepth_;
    int lastSeenSpawnEventId_ = -1;
};

}  // namespace fb::ai::beam
