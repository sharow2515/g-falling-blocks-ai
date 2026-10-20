#include "app/application_base.hpp"

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

ApplicationBase::ApplicationBase(const std::string& minosJsonPath, std::optional<std::uint64_t> seed,
                                 std::string windowTitle)
    : game_(MakeGame(minosJsonPath)), fixedSeed_(seed), windowTitle_(std::move(windowTitle)) {
    game_.StartGame(NextSeed());
}

std::uint64_t ApplicationBase::NextSeed() const {
    return fixedSeed_.has_value() ? *fixedSeed_ : RandomSeed();
}

bool ApplicationBase::TryRestart() {
    if (!game_.IsGameOver() || !input_.ConsumeRestartRequested()) return false;

    game_.RestartGame(NextSeed());
    return true;
}

int ApplicationBase::Run() {
    sf::RenderWindow window(sf::VideoMode({720u, 640u}), windowTitle_);
    window.setFramerateLimit(60);

    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float deltaSeconds = clock.restart().asSeconds();

        Update(deltaSeconds);

        window.clear(sf::Color::Black);
        renderer_.Draw(window, game_, deltaSeconds);
        window.display();
    }

    return 0;
}

}  // namespace fb::app
