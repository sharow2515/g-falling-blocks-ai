#pragma once

#include <string>
#include <vector>

#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai {

class SimplePlayer : public AiPlayer {
public:
    std::vector<PlayerMove> Plan(const core::Game& game) override;
    std::string Name() const override { return "Simple"; }
};

}  // namespace fb::ai
