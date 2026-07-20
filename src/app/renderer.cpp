#include "app/renderer.hpp"

#include <algorithm>
#include <iostream>
#include <limits>

namespace fb::app {

namespace {
sf::Color ToSfColor(core::Color c, std::uint8_t alpha = 255) {
    return sf::Color(c.r, c.g, c.b, alpha);
}
}  // namespace

Renderer::Renderer(float cellSize) : cellSize_(cellSize) {
    fontLoaded_ = font_.openFromFile("assets/fonts/DejaVuSans.ttf");
    if (!fontLoaded_) {
        std::cerr << "Renderer: フォントを読み込めませんでした(assets/fonts/DejaVuSans.ttf)。"
                     "テキスト表示をスキップします。\n";
    }
}

sf::Vector2f Renderer::BoardToScreen(core::Vec2i boardPos, const core::Board& board) const {
    float x = boardOrigin_.x + static_cast<float>(boardPos.x) * cellSize_;
    float y = boardOrigin_.y + static_cast<float>(board.Height() - 1 - boardPos.y) * cellSize_;
    return {x, y};
}

void Renderer::DrawCell(sf::RenderWindow& window, sf::Vector2f screenPos, sf::Color color) {
    sf::RectangleShape rect({cellSize_ - 1.f, cellSize_ - 1.f});
    rect.setPosition(screenPos);
    rect.setFillColor(color);
    window.draw(rect);
}

void Renderer::DrawBoard(sf::RenderWindow& window, const core::Board& board) {
    sf::RectangleShape border({cellSize_ * static_cast<float>(board.Width()),
                                cellSize_ * static_cast<float>(board.Height())});
    border.setPosition(boardOrigin_);
    border.setFillColor(sf::Color(20, 20, 20));
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2.f);
    window.draw(border);

    // マス目の視認性向上のための薄いグリッド線。
    sf::Color gridColor(50, 50, 50);
    for (int x = 0; x <= board.Width(); ++x) {
        sf::RectangleShape line({1.f, cellSize_ * static_cast<float>(board.Height())});
        line.setPosition({boardOrigin_.x + static_cast<float>(x) * cellSize_, boardOrigin_.y});
        line.setFillColor(gridColor);
        window.draw(line);
    }
    for (int y = 0; y <= board.Height(); ++y) {
        sf::RectangleShape line({cellSize_ * static_cast<float>(board.Width()), 1.f});
        line.setPosition({boardOrigin_.x, boardOrigin_.y + static_cast<float>(y) * cellSize_});
        line.setFillColor(gridColor);
        window.draw(line);
    }

    for (int y = 0; y < board.Height(); ++y) {
        for (int x = 0; x < board.Width(); ++x) {
            auto color = board.CellColor({x, y});
            if (color.has_value()) {
                DrawCell(window, BoardToScreen({x, y}, board), ToSfColor(*color));
            }
        }
    }
}

void Renderer::DrawGhost(sf::RenderWindow& window, const core::Game& game) {
    const auto& piece = game.GetActivePiece();
    if (!piece.IsActive()) return;

    auto cells = piece.GhostWorldCells(game.GetBoard());
    sf::Color color = ToSfColor(piece.MinoPtr()->color, 45);
    for (const auto& cell : cells) {
        if (cell.y < 0 || cell.y >= game.GetBoard().Height()) continue;
        DrawCell(window, BoardToScreen(cell, game.GetBoard()), color);
    }
}

void Renderer::DrawActivePiece(sf::RenderWindow& window, const core::Game& game) {
    const auto& piece = game.GetActivePiece();
    if (!piece.IsActive()) return;

    auto cells = piece.WorldCells();
    sf::Color color = ToSfColor(piece.MinoPtr()->color);
    for (const auto& cell : cells) {
        if (cell.y < 0 || cell.y >= game.GetBoard().Height()) continue;
        DrawCell(window, BoardToScreen(cell, game.GetBoard()), color);
    }
}

void Renderer::DrawMinoPreview(sf::RenderWindow& window, const core::Mino& mino, sf::Vector2f originTopLeft,
                                float previewCellSize) {
    auto cells = mino.GetCells(0);

    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        minX = std::min(minX, c.x);
        maxX = std::max(maxX, c.x);
        minY = std::min(minY, c.y);
        maxY = std::max(maxY, c.y);
    }

    // GameManager.DrawMinoPreview と同一の中心合わせ。
    float offsetX = (minX + maxX) / 2.f;
    float offsetY = (minY + maxY) / 2.f;

    sf::Color color = ToSfColor(mino.color);
    for (const auto& c : cells) {
        float localX = static_cast<float>(c.x) - offsetX;
        float localY = static_cast<float>(c.y) - offsetY;

        sf::RectangleShape rect({previewCellSize - 1.f, previewCellSize - 1.f});
        rect.setPosition({originTopLeft.x + (localX + 2.5f) * previewCellSize,
                           originTopLeft.y + (2.5f - localY) * previewCellSize});
        rect.setFillColor(color);
        window.draw(rect);
    }
}

void Renderer::DrawHold(sf::RenderWindow& window, const core::Game& game) {
    sf::RectangleShape box({cellSize_ * 5.f, cellSize_ * 4.f});
    box.setPosition(holdOrigin_);
    box.setFillColor(sf::Color(15, 15, 15));
    box.setOutlineColor(sf::Color(120, 120, 120));
    box.setOutlineThickness(1.f);
    window.draw(box);

    const core::Mino* hold = game.GetHoldMino();
    if (hold != nullptr) {
        DrawMinoPreview(window, *hold, holdOrigin_, cellSize_ * 0.6f);
    }
}

void Renderer::DrawNext(sf::RenderWindow& window, const core::Game& game) {
    auto nextMinos = game.PeekNext(5);
    for (std::size_t i = 0; i < nextMinos.size(); ++i) {
        sf::Vector2f origin = nextOrigin_ + sf::Vector2f{0.f, static_cast<float>(i) * cellSize_ * 3.2f};

        sf::RectangleShape box({cellSize_ * 5.f, cellSize_ * 3.f});
        box.setPosition(origin);
        box.setFillColor(sf::Color(15, 15, 15));
        box.setOutlineColor(sf::Color(120, 120, 120));
        box.setOutlineThickness(1.f);
        window.draw(box);

        DrawMinoPreview(window, *nextMinos[i], origin, cellSize_ * 0.5f);
    }
}

void Renderer::DrawText(sf::RenderWindow& window, const std::string& text, sf::Vector2f pos, unsigned int charSize,
                         sf::Color color) {
    if (!fontLoaded_) return;

    sf::Text sfText(font_, text, charSize);
    sfText.setPosition(pos);
    sfText.setFillColor(color);
    window.draw(sfText);
}

void Renderer::DrawStats(sf::RenderWindow& window, const core::Game& game) {
    DrawText(window, "SCORE", statsOrigin_, 16, sf::Color(160, 160, 160));
    DrawText(window, std::to_string(game.GetScore()), statsOrigin_ + sf::Vector2f{0.f, 20.f}, 24, sf::Color::White);

    DrawText(window, "LEVEL", statsOrigin_ + sf::Vector2f{0.f, 60.f}, 16, sf::Color(160, 160, 160));
    DrawText(window, std::to_string(game.GetLevel()), statsOrigin_ + sf::Vector2f{0.f, 80.f}, 24, sf::Color::White);

    DrawText(window, "LINES", statsOrigin_ + sf::Vector2f{0.f, 120.f}, 16, sf::Color(160, 160, 160));
    DrawText(window, std::to_string(game.GetTotalLines()), statsOrigin_ + sf::Vector2f{0.f, 140.f}, 24,
             sf::Color::White);
}

void Renderer::DrawGameOverOverlay(sf::RenderWindow& window, const core::Game& game) {
    sf::RectangleShape overlay(static_cast<sf::Vector2f>(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    sf::Vector2f center{window.getSize().x / 2.f - 90.f, window.getSize().y / 2.f - 40.f};
    DrawText(window, "GAME OVER", center, 32, sf::Color::White);
    DrawText(window, "Score: " + std::to_string(game.GetScore()), center + sf::Vector2f{0.f, 44.f}, 20,
             sf::Color::White);
    DrawText(window, "Press R to restart", center + sf::Vector2f{0.f, 76.f}, 16, sf::Color(200, 200, 200));
}

void Renderer::UpdateLineClearFlash(const core::Game& game, float dt) {
    int id = game.ClearEventId();
    if (id != lastSeenClearEventId_) {
        lastSeenClearEventId_ = id;
        flashRows_ = game.LastClearedRows();
        flashTimer_ = kFlashDuration;
    } else if (flashTimer_ > 0.f) {
        flashTimer_ = std::max(0.f, flashTimer_ - dt);
    }
}

void Renderer::DrawLineClearFlash(sf::RenderWindow& window, const core::Board& board) {
    if (flashTimer_ <= 0.f) return;

    auto alpha = static_cast<std::uint8_t>(220.f * (flashTimer_ / kFlashDuration));
    sf::Color flashColor(255, 255, 255, alpha);

    for (int y : flashRows_) {
        if (y < 0 || y >= board.Height()) continue;

        sf::RectangleShape rect({cellSize_ * static_cast<float>(board.Width()), cellSize_});
        rect.setPosition(BoardToScreen({0, y}, board));
        rect.setFillColor(flashColor);
        window.draw(rect);
    }
}

void Renderer::Draw(sf::RenderWindow& window, const core::Game& game, float dt) {
    UpdateLineClearFlash(game, dt);

    DrawBoard(window, game.GetBoard());
    DrawLineClearFlash(window, game.GetBoard());
    DrawGhost(window, game);
    DrawActivePiece(window, game);
    DrawHold(window, game);
    DrawNext(window, game);
    DrawStats(window, game);

    if (game.IsGameOver()) {
        DrawGameOverOverlay(window, game);
    }
}

}  // namespace fb::app
