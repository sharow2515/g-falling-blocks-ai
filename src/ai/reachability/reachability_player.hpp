#pragma once

#include <string>
#include <vector>

#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai::reachability {

class ReachabilityPlayer : public AiPlayer {
public:
    std::vector<PlayerMove> Plan(const core::Game& game) override;
    std::string Name() const override { return "Reachability"; }
};

}  // namespace fb::ai::reachability
