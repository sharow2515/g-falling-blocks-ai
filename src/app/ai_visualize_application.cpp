#include "app/ai_visualize_application.hpp"

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

AiVisualizeApplication::AiVisualizeApplication(const std::string& minosJsonPath,
                                              std::unique_ptr<ai::AiPlayer> aiPlayer, float moveDelay,
                                              std::optional<std::uint64_t> seed)
    : game_(MakeGame(minosJsonPath)), ai_(std::move(aiPlayer)), moveDelay_(moveDelay), fixedSeed_(seed) {
    game_.StartGame(NextSeed());
}

std::uint64_t AiVisualizeApplication::NextSeed() const {
    return fixedSeed_.has_value() ? *fixedSeed_ : RandomSeed();
}

void AiVisualizeApplication::ExecuteMove(ai::PlayerMove move) {
    switch (move) {
        case ai::PlayerMove::Left: game_.MoveLeft(); break;
        case ai::PlayerMove::Right: game_.MoveRight(); break;
        case ai::PlayerMove::Down: game_.MoveDown(); break;
        case ai::PlayerMove::RotateCW: game_.RotateCW(); break;
        case ai::PlayerMove::RotateCCW: game_.RotateCCW(); break;
        case ai::PlayerMove::Hold: game_.Hold(); break;
        case ai::PlayerMove::HardDrop: game_.HardDrop(); break;
    }
}

int AiVisualizeApplication::Run() {
    sf::RenderWindow window(sf::VideoMode({720u, 640u}), "FallingBlocks - AI Visualize");
    window.setFramerateLimit(60);

    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        bool restartPressed = sf::Keyboard::isKeyPressed(DefaultKeyFor(KeyAction::Restart));
        bool restartJustPressed = restartPressed && !prevRestartPressed_;
        prevRestartPressed_ = restartPressed;

        if (game_.IsGameOver()) {
            if (restartJustPressed) {
                game_.RestartGame(NextSeed());
                pendingMoves_.clear();
                cooldownTimer_ = 0.f;
            }
        } else {
            cooldownTimer_ -= dt;
            if (cooldownTimer_ <= 0.f) {
                if (pendingMoves_.empty()) {
                    pendingMoves_ = ai_->Plan(game_);
                }
                if (!pendingMoves_.empty()) {
                    ai::PlayerMove move = pendingMoves_.front();
                    pendingMoves_.erase(pendingMoves_.begin());
                    ExecuteMove(move);
                }
                cooldownTimer_ = moveDelay_;
            }
        }
        // core::Game::Tick()は意図的に呼ばない(ヘッダのコメント参照)。

        window.clear(sf::Color::Black);
        renderer_.Draw(window, game_, dt);
        window.display();
    }

    return 0;
}

}  // namespace fb::app
