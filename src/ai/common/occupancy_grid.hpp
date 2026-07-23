#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "core/board.hpp"
#include "core/types.hpp"

namespace fb::ai::common {

// core::Boardの占有状態をビットマスク化したもの。色は持たない(AIの評価に色は不要なため)。
// 2通りの使い方がある:
//   1. board.CellColor()を何度も呼ぶ代わりに使う、関数ローカルの読み取り専用スナップショット
//      (探索中にboardは変化しない前提)。
//   2. PlacePiece/ClearFullLinesで自ら状態を更新しながら、core::Boardを介さずに
//      持ち回る軽量な盤面表現(ai::beamのBeamNodeなど)。
//
// ComputeFeatures等、候補配置ごとに毎回構築/コピーされる呼ばれ方をするため、
// ヒープ確保を避けて固定長のスタック配列で持つ(盤面の高さは実行中変わらない)。
class OccupancyGrid {
public:
    static constexpr int kMaxRows = 64;

    // 幅0・高さ0の空の状態。BeamNodeのデフォルト構築のためだけに存在し、
    // 実際に使う前に必ずexplicit OccupancyGrid(const core::Board&)で作った値で上書きされる。
    OccupancyGrid() = default;

    explicit OccupancyGrid(const core::Board& board);

    [[nodiscard]] bool IsInsideBoard(core::Vec2i pos) const {
        return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
    }

    [[nodiscard]] bool IsOccupied(core::Vec2i pos) const {
        return ((rows_[static_cast<std::size_t>(pos.y)] >> pos.x) & 1u) != 0;
    }

    // y行目の生のビット列(bit x = 列xが埋まっているか)。ビット演算で複数セルを
    // まとめて扱いたい特徴量計算のために公開している。
    [[nodiscard]] std::uint32_t RowBits(int y) const { return rows_[static_cast<std::size_t>(y)]; }

    [[nodiscard]] bool IsValidPosition(const std::vector<core::Vec2i>& cells, core::Vec2i position) const {
        for (const auto& cell : cells) {
            core::Vec2i boardPos = position + cell;
            if (!IsInsideBoard(boardPos)) return false;
            if (IsOccupied(boardPos)) return false;
        }
        return true;
    }

    // cellsの示すセルを埋める(範囲外は無視。core::Board::PlacePieceと同じ意味論)。
    void PlacePiece(const std::vector<core::Vec2i>& cells, core::Vec2i position) {
        for (const auto& cell : cells) {
            core::Vec2i pos = position + cell;
            if (IsInsideBoard(pos)) {
                rows_[static_cast<std::size_t>(pos.y)] |= (1u << pos.x);
            }
        }
    }

    // 揃った行(全列埋まっている行)を消去し、消去前の行番号をclearedRowsに積む(下から順)。
    // 消去数を返す。core::Board::ClearFullLinesと同じ意味論。
    int ClearFullLines(std::vector<int>& clearedRows) {
        clearedRows.clear();
        const std::uint32_t fullMask = (width_ >= 32) ? 0xFFFFFFFFu : ((1u << width_) - 1u);

        for (int y = 0; y < height_; ++y) {
            if (rows_[static_cast<std::size_t>(y)] == fullMask) clearedRows.push_back(y);
        }

        const int linesCleared = static_cast<int>(clearedRows.size());
        for (std::size_t i = 0; i < clearedRows.size(); ++i) {
            int y = clearedRows[i] - static_cast<int>(i);
            for (int shiftY = y; shiftY < height_ - 1; ++shiftY) {
                rows_[static_cast<std::size_t>(shiftY)] = rows_[static_cast<std::size_t>(shiftY + 1)];
            }
            rows_[static_cast<std::size_t>(height_ - 1)] = 0;
        }
        return linesCleared;
    }

    // core::Board::IsGameOverと同じ意味論(見えている範囲のすぐ上の行に何かあるか)。
    [[nodiscard]] bool IsGameOver() const { return rows_[static_cast<std::size_t>(visibleHeight_)] != 0; }

    [[nodiscard]] int Width() const { return width_; }
    // extraHeight分のバッファ域を含む全高(BFSの範囲判定用)。
    [[nodiscard]] int TotalHeight() const { return height_; }
    // 見えている範囲だけの高さ(特徴量計算のスキャン範囲・スポーン位置計算用)。
    [[nodiscard]] int VisibleHeight() const { return visibleHeight_; }

private:
    int width_ = 0;
    int height_ = 0;
    int visibleHeight_ = 0;
    std::array<std::uint32_t, kMaxRows> rows_{};  // 1行1ワード。bit x = 列x。行インデックス = y。
};

}  // namespace fb::ai::common
