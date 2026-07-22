#include "ai/common/path_conversion.hpp"

namespace fb::ai::common {

std::vector<PlayerMove> ConvertPath(const std::vector<MoveType>& path) {
    std::vector<MoveType> trimmed = path;
    while (!trimmed.empty() && trimmed.back() == MoveType::Down) {
        trimmed.pop_back();
    }

    std::vector<PlayerMove> plan;
    plan.reserve(trimmed.size() + 1);
    for (MoveType move : trimmed) {
        switch (move) {
            case MoveType::Left: plan.push_back(PlayerMove::Left); break;
            case MoveType::Right: plan.push_back(PlayerMove::Right); break;
            case MoveType::Down: plan.push_back(PlayerMove::Down); break;
            case MoveType::RotateCW: plan.push_back(PlayerMove::RotateCW); break;
            case MoveType::RotateCCW: plan.push_back(PlayerMove::RotateCCW); break;
        }
    }
    plan.push_back(PlayerMove::HardDrop);
    return plan;
}

}  // namespace fb::ai::common
