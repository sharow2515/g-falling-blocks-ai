#include "core/board.hpp"

namespace fb::core {

Board::Board(int width, int height, int extraHeight)
    : width_(width),
      height_(height),
      extraHeight_(extraHeight),
      grid_(static_cast<std::size_t>(width) * (height + extraHeight)) {}

bool Board::IsValidPosition(const std::vector<Vec2i>& cells, Vec2i position) const {
    for (const auto& cell : cells) {
        Vec2i boardPos = position + cell;

        if (!IsInsideBoard(boardPos)) return false;
        if (grid_[Index(boardPos.x, boardPos.y)].has_value()) return false;
    }
    return true;
}

void Board::PlacePiece(const std::vector<Vec2i>& cells, Vec2i position, Color color) {
    for (const auto& cell : cells) {
        Vec2i boardPos = position + cell;
        if (boardPos.x >= 0 && boardPos.x < width_ && boardPos.y >= 0 && boardPos.y < TotalHeight()) {
            grid_[Index(boardPos.x, boardPos.y)] = color;
        }
    }
}

int Board::ClearFullLines() {
    std::vector<int> clearedRows;
    return ClearFullLines(clearedRows);
}

int Board::ClearFullLines(std::vector<int>& clearedRows) {
    clearedRows.clear();

    // まず消える行を記録
    for (int y = 0; y < TotalHeight(); ++y) {
        if (IsLineFull(y)) {
            clearedRows.push_back(y);
        }
    }

    int linesCleared = static_cast<int>(clearedRows.size());

    // 下の行から順に消去（インデックスずれ考慮）
    for (std::size_t i = 0; i < clearedRows.size(); ++i) {
        int y = clearedRows[i] - static_cast<int>(i);
        ClearLine(y);
        ShiftLinesDown(y);
    }

    return linesCleared;
}

bool Board::IsLineFull(int y) const {
    for (int x = 0; x < width_; ++x) {
        if (!grid_[Index(x, y)].has_value()) return false;
    }
    return true;
}

void Board::ClearLine(int y) {
    for (int x = 0; x < width_; ++x) {
        grid_[Index(x, y)] = std::nullopt;
    }
}

void Board::ShiftLinesDown(int clearedLine) {
    for (int y = clearedLine; y < TotalHeight() - 1; ++y) {
        for (int x = 0; x < width_; ++x) {
            grid_[Index(x, y)] = grid_[Index(x, y + 1)];
        }
    }

    for (int x = 0; x < width_; ++x) {
        grid_[Index(x, TotalHeight() - 1)] = std::nullopt;
    }
}

int Board::GetGhostY(const std::vector<Vec2i>& cells, Vec2i position) const {
    Vec2i ghostPos = position;

    while (IsValidPosition(cells, ghostPos + Vec2i::Down())) {
        ghostPos = ghostPos + Vec2i::Down();
    }

    return ghostPos.y;
}

bool Board::IsGameOver() const {
    for (int x = 0; x < width_; ++x) {
        if (grid_[Index(x, height_)].has_value()) return true;
    }
    return false;
}

void Board::ClearBoard() {
    grid_.assign(static_cast<std::size_t>(width_) * TotalHeight(), std::nullopt);
}

int Board::CountTSpinCorners(Vec2i centerPos) const {
    int count = 0;
    const Vec2i corners[] = {
        {centerPos.x - 1, centerPos.y + 1},  // 左上
        {centerPos.x + 1, centerPos.y + 1},  // 右上
        {centerPos.x - 1, centerPos.y - 1},  // 左下
        {centerPos.x + 1, centerPos.y - 1},  // 右下
    };

    for (const auto& corner : corners) {
        if (IsCellOccupied(corner)) ++count;
    }
    return count;
}

}  // namespace fb::core
