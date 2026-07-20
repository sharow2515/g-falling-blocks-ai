#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "core/game.hpp"

namespace fb::app {

class Renderer {
public:
    explicit Renderer(float cellSize = 28.f);

    void Draw(sf::RenderWindow& window, const core::Game& game, float dt);

private:
    void DrawBoard(sf::RenderWindow& window, const core::Board& board);
    void DrawGhost(sf::RenderWindow& window, const core::Game& game);
    void DrawActivePiece(sf::RenderWindow& window, const core::Game& game);
    void DrawHold(sf::RenderWindow& window, const core::Game& game);
    void DrawNext(sf::RenderWindow& window, const core::Game& game);
    void DrawMinoPreview(sf::RenderWindow& window, const core::Mino& mino, sf::Vector2f originTopLeft,
                          float previewCellSize);
    void DrawCell(sf::RenderWindow& window, sf::Vector2f screenPos, sf::Color color);
    void DrawStats(sf::RenderWindow& window, const core::Game& game);
    void DrawGameOverOverlay(sf::RenderWindow& window, const core::Game& game);
    void DrawText(sf::RenderWindow& window, const std::string& text, sf::Vector2f pos, unsigned int charSize,
                  sf::Color color);

    void UpdateLineClearFlash(const core::Game& game, float dt);
    void DrawLineClearFlash(sf::RenderWindow& window, const core::Board& board);

    [[nodiscard]] sf::Vector2f BoardToScreen(core::Vec2i boardPos, const core::Board& board) const;

    float cellSize_;
    sf::Vector2f boardOrigin_{160.f, 20.f};
    sf::Vector2f holdOrigin_{20.f, 20.f};
    sf::Vector2f nextOrigin_{480.f, 20.f};
    sf::Vector2f statsOrigin_{20.f, 260.f};

    sf::Font font_;
    bool fontLoaded_ = false;

    int lastSeenClearEventId_ = 0;
    std::vector<int> flashRows_;
    float flashTimer_ = 0.f;
    static constexpr float kFlashDuration = 0.15f;
};

}  // namespace fb::app
