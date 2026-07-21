#pragma once

#include <string>

#include "ai/heuristic/heuristic_player.hpp"
#include "app/input_controller.hpp"
#include "app/renderer.hpp"
#include "core/game.hpp"

namespace fb::app {

class Application {
public:
    // minosJsonPath: 読み込むミノ定義ファイル(例: "assets/minos_hard.json")。
    explicit Application(const std::string& minosJsonPath);

    int Run();

private:
    core::Game game_;
    InputController input_;
    Renderer renderer_;
    ai::HeuristicPlayer ai_;
    bool aiEnabled_ = false;
};

}  // namespace fb::app
