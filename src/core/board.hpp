#pragma once

#include <optional>
#include <vector>

#include "core/types.hpp"

namespace fb::core {

class Board {
public:
    explicit Board(int width = 10, int height = 20, int extraHeight = 4);

    int Width() const { return width_; }
    int Height() const { return height_; }
    int ExtraHeight() const { return extraHeight_; }
    int TotalHeight() const { return height_ + extraHeight_; }

    [[nodiscard]] bool IsInsideBoard(Vec2i pos) const;
    [[nodiscard]] bool IsValidPosition(const std::vector<Vec2i>& cells, Vec2i position) const;

    void PlacePiece(const std::vector<Vec2i>& cells, Vec2i position, Color color);

    // 揃った行を消去し、消した行数を返す。
    int ClearFullLines();

    // 揃った行を消去し、消した行数と消えた行番号リスト(消去前のy)を返す。
    int ClearFullLines(std::vector<int>& clearedRows);

    [[nodiscard]] int GetGhostY(const std::vector<Vec2i>& cells, Vec2i position) const;
    [[nodiscard]] bool IsGameOver() const;

    void ClearBoard();

    // Tスピン判定用: 中心位置の4隅のブロック有無を調べる。埋まっているコーナー数(0-4)を返す。
    [[nodiscard]] int CountTSpinCorners(Vec2i centerPos) const;

    [[nodiscard]] bool IsCellOccupied(Vec2i pos) const;
    [[nodiscard]] std::optional<Color> CellColor(Vec2i pos) const;

private:
    [[nodiscard]] bool IsLineFull(int y) const;
    void ClearLine(int y);
    void ShiftLinesDown(int clearedLine);

    [[nodiscard]] std::size_t Index(int x, int y) const { return static_cast<std::size_t>(y) * width_ + x; }

    int width_;
    int height_;
    int extraHeight_;
    std::vector<std::optional<Color>> grid_;  // index = y*width_+x. null=空。
};

}  // namespace fb::core
