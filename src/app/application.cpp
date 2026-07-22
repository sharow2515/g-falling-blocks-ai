#include "app/application.hpp"

#include <random>

#include "core/mino_loader.hpp"

namespace fb::app {

namespace {

core::Game MakeGame(const std::string& minosJsonPath) {
    auto minos = core::LoadMinos(minosJsonPath);
    return core::Game(std::move(minos));
}

std::uint64_t RandomSeed() {
    std::random_device rd;
    return (static_cast<std::uint64_t>(rd()) << 32) | rd();
}

}  // namespace

Application::Application(const std::string& minosJsonPath, std::unique_ptr<ai::AiPlayer> aiPlayer,
                          std::optional<std::uint64_t> seed)
    : game_(MakeGame(minosJsonPath)), ai_(std::move(aiPlayer)), fixedSeed_(seed) {
    game_.StartGame(NextSeed());
}

std::uint64_t Application::NextSeed() const {
    return fixedSeed_.has_value() ? *fixedSeed_ : RandomSeed();
}

int Application::Run() {
    sf::RenderWindow window(sf::VideoMode({720u, 640u}), "FallingBlocks");
    window.setFramerateLimit(60);

    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        input_.Update(game_, dt, /*controlPiece=*/!aiEnabled_);
        if (input_.ConsumeToggleAiRequested()) {
            aiEnabled_ = !aiEnabled_;
        }
        if (game_.IsGameOver() && input_.ConsumeRestartRequested()) {
            game_.RestartGame(NextSeed());
        }

        if (aiEnabled_) {
            ai_->Update(game_);
        }

        game_.Tick(dt);

        window.clear(sf::Color::Black);
        renderer_.Draw(window, game_, dt);
        window.display();
    }

    return 0;
}

}  // namespace fb::app
