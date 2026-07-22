#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "ai/player.hpp"
#include "app/input_controller.hpp"
#include "app/renderer.hpp"
#include "core/game.hpp"

namespace fb::app {

class Application {
public:
    // minosJsonPath: 読み込むミノ定義ファイル(例: "assets/minos_hard.json")。
    // aiPlayer: Aキーでトグルする自動プレイに使うAI(例: SimplePlayer/ReachabilityPlayer)。
    // seed: 指定するとミノの出現列が再現可能になる。Restartキーでも同じシードを使い直す
    Application(const std::string& minosJsonPath, std::unique_ptr<ai::AiPlayer> aiPlayer,
                std::optional<std::uint64_t> seed = std::nullopt);

    int Run();

private:
    std::uint64_t NextSeed() const;

    core::Game game_;
    InputController input_;
    Renderer renderer_;
    std::unique_ptr<ai::AiPlayer> ai_;
    bool aiEnabled_ = false;

    std::optional<std::uint64_t> fixedSeed_;
};

}  // namespace fb::app
