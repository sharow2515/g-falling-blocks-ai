#include "app/input_controller.hpp"

namespace fb::app {

bool InputController::WasPressedThisFrame(KeyAction action) const {
    auto i = static_cast<std::size_t>(action);
    return currPressed_[i] && !prevPressed_[i];
}

bool InputController::IsPressed(KeyAction action) const {
    return currPressed_[static_cast<std::size_t>(action)];
}

void InputController::HandleHorizontalMovement(core::Game& game, float deltaSeconds) {
    // 左移動
    if (WasPressedThisFrame(KeyAction::MoveLeft)) {
        game.MoveLeft();
        dasTimerLeft_ = 0.f;
        dasActiveLeft_ = false;
        arrTimerLeft_ = 0.f;
    } else if (IsPressed(KeyAction::MoveLeft)) {
        dasTimerLeft_ += deltaSeconds;
        if (!dasActiveLeft_ && dasTimerLeft_ >= dasDelay_) {
            dasActiveLeft_ = true;
            arrTimerLeft_ = 0.f;
            game.MoveLeft();
        } else if (dasActiveLeft_) {
            arrTimerLeft_ += deltaSeconds;
            while (arrTimerLeft_ >= arrInterval_) {
                game.MoveLeft();
                arrTimerLeft_ -= arrInterval_;
            }
        }
    } else {
        dasTimerLeft_ = 0.f;
        dasActiveLeft_ = false;
    }

    // 右移動
    if (WasPressedThisFrame(KeyAction::MoveRight)) {
        game.MoveRight();
        dasTimerRight_ = 0.f;
        dasActiveRight_ = false;
        arrTimerRight_ = 0.f;
    } else if (IsPressed(KeyAction::MoveRight)) {
        dasTimerRight_ += deltaSeconds;
        if (!dasActiveRight_ && dasTimerRight_ >= dasDelay_) {
            dasActiveRight_ = true;
            arrTimerRight_ = 0.f;
            game.MoveRight();
        } else if (dasActiveRight_) {
            arrTimerRight_ += deltaSeconds;
            while (arrTimerRight_ >= arrInterval_) {
                game.MoveRight();
                arrTimerRight_ -= arrInterval_;
            }
        }
    } else {
        dasTimerRight_ = 0.f;
        dasActiveRight_ = false;
    }
}

void InputController::ResetDasArrIfNewPiece(const core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;

    lastSeenSpawnEventId_ = spawnId;
    dasTimerLeft_ = 0.f;
    dasTimerRight_ = 0.f;
    arrTimerLeft_ = 0.f;
    arrTimerRight_ = 0.f;
    dasActiveLeft_ = false;
    dasActiveRight_ = false;
}

void InputController::Update(core::Game& game, float deltaSeconds, bool controlPiece) {
    for (std::size_t i = 0; i < kActionCount; ++i) {
        currPressed_[i] = sf::Keyboard::isKeyPressed(DefaultKeyFor(static_cast<KeyAction>(i)));
    }

    if (controlPiece) {
        ResetDasArrIfNewPiece(game);
        HandleHorizontalMovement(game, deltaSeconds);

        if (WasPressedThisFrame(KeyAction::RotateCW)) game.RotateCW();
        if (WasPressedThisFrame(KeyAction::RotateCCW)) game.RotateCCW();

        if (WasPressedThisFrame(KeyAction::HardDrop)) {
            game.HardDrop();
        } else {
            game.SetSoftDropping(IsPressed(KeyAction::SoftDrop));
            if (WasPressedThisFrame(KeyAction::Hold)) game.Hold();
        }
    }

    if (!game.IsGameOver() && WasPressedThisFrame(KeyAction::Pause)) game.TogglePause();

    restartRequested_ = WasPressedThisFrame(KeyAction::Restart);

    prevPressed_ = currPressed_;
}

bool InputController::ConsumeRestartRequested() {
    bool requested = restartRequested_;
    restartRequested_ = false;
    return requested;
}

}  // namespace fb::app
