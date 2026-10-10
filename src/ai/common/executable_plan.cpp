#include "ai/common/executable_plan.hpp"

namespace fb::ai::common {

std::vector<PlayerMove> ToExecutablePlan(const std::vector<PlayerMove>& path) {
    std::vector<PlayerMove> plan = path;
    while (!plan.empty() && plan.back() == PlayerMove::Down) {
        plan.pop_back();
    }
    plan.push_back(PlayerMove::HardDrop);
    return plan;
}

}  // namespace fb::ai::common
