#pragma once

#include "app/input_controller.hpp"
#include "app/renderer.hpp"
#include "core/game.hpp"

namespace fb::app {

class Application {
public:
    Application();

    int Run();

private:
    core::Game game_;
    InputController input_;
    Renderer renderer_;
};

}  // namespace fb::app
