#include "ai/common/move_execution.hpp"

namespace fb::ai::common {

void ExecuteMove(core::Game& game, PlayerMove move) {
    switch (move) {
        case PlayerMove::Left: game.MoveLeft(); break;
        case PlayerMove::Right: game.MoveRight(); break;
        case PlayerMove::Down: game.MoveDown(); break;
        case PlayerMove::RotateCW: game.RotateCW(); break;
        case PlayerMove::RotateCCW: game.RotateCCW(); break;
        case PlayerMove::Hold: game.Hold(); break;
        case PlayerMove::HardDrop: game.HardDrop(); break;
    }
}

}  // namespace fb::ai::common
