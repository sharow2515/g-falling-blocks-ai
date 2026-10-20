#pragma once

#include <array>

#include "app/key_config.hpp"
#include "core/game.hpp"

namespace fb::app {

class InputController {
public:
    // 毎フレーム呼び出し、DAS/ARRを進めてcore::Gameへコマンドを発行する。
    // controlPiece=false の間は移動・回転・ソフト/ハードドロップ・ホールドを発行しない
    // (AIが操作するモードで人間の入力と競合しないようにするため)。
    // Pause/RestartキーはcontrolPieceの値に関わらず常に処理する。
    void Update(core::Game& game, float deltaSeconds, bool controlPiece = true);

    [[nodiscard]] bool ConsumeRestartRequested();

private:
    [[nodiscard]] bool WasPressedThisFrame(KeyAction action) const;
    [[nodiscard]] bool IsPressed(KeyAction action) const;
    void HandleHorizontalMovement(core::Game& game, float deltaSeconds);
    void ResetDasArrIfNewPiece(const core::Game& game);

    static constexpr std::size_t kActionCount = static_cast<std::size_t>(KeyAction::Count);
    std::array<bool, kActionCount> prevPressed_{};
    std::array<bool, kActionCount> currPressed_{};

    float dasDelay_ = 0.17f;
    float arrInterval_ = 0.05f;

    float dasTimerLeft_ = 0.f;
    float dasTimerRight_ = 0.f;
    float arrTimerLeft_ = 0.f;
    float arrTimerRight_ = 0.f;
    bool dasActiveLeft_ = false;
    bool dasActiveRight_ = false;

    int lastSeenSpawnEventId_ = -1;

    bool restartRequested_ = false;
};

}  // namespace fb::app
