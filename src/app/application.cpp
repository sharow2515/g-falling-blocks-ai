#include "app/application.hpp"

#include <random>

#include "core/mino_loader.hpp"

namespace fb::app {

namespace {

core::Game MakeGame() {
    auto minos = core::LoadMinos("assets/minos.json");
    return core::Game(std::move(minos));
}

std::uint64_t RandomSeed() {
    std::random_device rd;
    return (static_cast<std::uint64_t>(rd()) << 32) | rd();
}

}  // namespace

Application::Application() : game_(MakeGame()) {
    game_.StartGame(RandomSeed());
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
            game_.RestartGame(RandomSeed());
        }

        if (aiEnabled_) {
            ai_.Update(game_);
        }

        game_.Tick(dt);

        window.clear(sf::Color::Black);
        renderer_.Draw(window, game_, dt);
        window.display();
    }

    return 0;
}

}  // namespace fb::app
