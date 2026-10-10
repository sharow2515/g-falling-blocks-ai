#pragma once

#include <string>
#include <vector>

#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai::reachability {

// 実際の移動・回転(ウォールキック含む)で到達可能な全ての置き場所をBFSで探索し、
// 評価関数で採点して最善のものを選ぶAI。
class ReachabilityPlayer : public AiPlayer {
public:
    std::vector<PlayerMove> Plan(core::Game& game) override;
    std::string Name() const override { return "Reachability"; }

private:
    std::vector<PlayerMove> BuildPlan(core::Game& game);
};

}  // namespace fb::ai::reachability
