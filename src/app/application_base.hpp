#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "app/input_controller.hpp"
#include "app/renderer.hpp"
#include "core/game.hpp"

namespace fb::app {

class ApplicationBase {
public:
    virtual ~ApplicationBase() = default;

    int Run();

protected:
    ApplicationBase(const std::string& minosJsonPath, std::optional<std::uint64_t> seed, std::string windowTitle);

    // 毎フレーム、描画の直前に呼ばれる。
    virtual void Update(float deltaSeconds) = 0;

    bool TryRestart();

    core::Game game_;
    InputController input_;
    Renderer renderer_;

private:
    std::uint64_t NextSeed() const;

    std::optional<std::uint64_t> fixedSeed_;
    std::string windowTitle_;
};

}  // namespace fb::app
