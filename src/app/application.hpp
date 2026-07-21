#pragma once

#include <memory>
#include <string>

#include "ai/player.hpp"
#include "app/input_controller.hpp"
#include "app/renderer.hpp"
#include "core/game.hpp"

namespace fb::app {

class Application {
public:
    // minosJsonPath: 読み込むミノ定義ファイル(例: "assets/minos_hard.json")。
    // aiPlayer: Aキーでトグルする自動プレイに使うAI(例: HeuristicPlayer/ReachabilityPlayer)。
    Application(const std::string& minosJsonPath, std::unique_ptr<ai::AiPlayer> aiPlayer);

    int Run();

private:
    core::Game game_;
    InputController input_;
    Renderer renderer_;
    std::unique_ptr<ai::AiPlayer> ai_;
    bool aiEnabled_ = false;
};

}  // namespace fb::app
