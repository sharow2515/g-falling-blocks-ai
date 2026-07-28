#include "ai/common/board_features.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace fb::ai::common {

double ComputeLandingHeight(const std::vector<core::Vec2i>& cells) {
    int minRow = std::numeric_limits<int>::max();
    int maxRow = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        minRow = std::min(minRow, c.y);
        maxRow = std::max(maxRow, c.y);
    }
    return (minRow + maxRow) / 2.0;
}

double ComputeErodedPieceCells(const std::vector<core::Vec2i>& cells, const std::vector<int>& clearedRows) {
    int erodedCellCount = 0;
    for (int row : clearedRows) {
        for (const auto& c : cells) {
            if (c.y == row) ++erodedCellCount;
        }
    }
    return static_cast<double>(clearedRows.size()) * static_cast<double>(erodedCellCount);
}

int CountHoles(const OccupancyGrid& grid) {
    // ColBits()はバッファ域も含む全高分のビットを持つため、見えている範囲だけに絞る。
    const std::uint32_t visibleMask = (1u << grid.VisibleHeight()) - 1u;

    // 列の最上位ビットより下にある0ビットの数 = (最上位ビットの位置) - (それより下の1ビットの数)。
    int holes = 0;
    for (int x = 0; x < grid.Width(); ++x) {
        std::uint32_t col = grid.ColBits(x) & visibleMask;
        if (col == 0) continue;  // 何も埋まっていない列に穴はない
        int highestBit = 31 - __builtin_clz(col);
        std::uint32_t belowMask = (1u << highestBit) - 1u;
        holes += highestBit - __builtin_popcount(col & belowMask);
    }
    return holes;
}

int CountRowTransitions(const OccupancyGrid& grid) {
    const int width = grid.Width();
    assert(width <= 30 && "この関数のビット演算はwidth+2ビットがuint32_tに収まる範囲でのみ正しい");

    // 「左壁(埋)+行のビット列+右壁(埋)」というwidth+2ビットの値を作り、
    // 隣接ビットの不一致数(width+1回の比較)を1回のpopcountで数える。
    const std::uint32_t compareMask = (1u << (width + 1)) - 1u;

    int transitions = 0;
    for (int y = 0; y < grid.VisibleHeight(); ++y) {
        std::uint32_t extended = 1u | (grid.RowBits(y) << 1) | (1u << (width + 1));
        transitions += __builtin_popcount((extended ^ (extended >> 1)) & compareMask);
    }
    return transitions;
}

int CountColumnTransitions(const OccupancyGrid& grid) {
    const int visibleHeight = grid.VisibleHeight();
    assert(visibleHeight <= 30 && "この関数のビット演算はvisibleHeight+1ビットがuint32_tに収まる範囲でのみ正しい");

    // 「床(埋)+列のビット列」というvisibleHeight+1ビットの値を作り、
    // 隣接ビットの不一致数(visibleHeight回の比較)を1回のpopcountで数える。
    // 天井は壁扱いしないので、行のときと違い上端のセンチネルビットは付けない。
    const std::uint32_t compareMask = (1u << visibleHeight) - 1u;

    int transitions = 0;
    for (int x = 0; x < grid.Width(); ++x) {
        std::uint32_t extended = 1u | (grid.ColBits(x) << 1);
        transitions += __builtin_popcount((extended ^ (extended >> 1)) & compareMask);
    }
    return transitions;
}

namespace {
// 列xの高さ = 最上段の埋まっているビット位置+1(空列は0)。CountHolesと同じ考え方。
int ColumnHeight(const OccupancyGrid& grid, std::uint32_t visibleMask, int x) {
    std::uint32_t col = grid.ColBits(x) & visibleMask;
    if (col == 0) return 0;
    return 32 - __builtin_clz(col);
}
}  // namespace

int ComputeBumpiness(const OccupancyGrid& grid) {
    const int width = grid.Width();
    const std::uint32_t visibleMask = (1u << grid.VisibleHeight()) - 1u;

    int bumpiness = 0;
    int prevHeight = ColumnHeight(grid, visibleMask, 0);
    for (int x = 1; x < width; ++x) {
        int height = ColumnHeight(grid, visibleMask, x);
        bumpiness += std::abs(height - prevHeight);
        prevHeight = height;
    }
    return bumpiness;
}

int ComputeAggregateHeight(const OccupancyGrid& grid) {
    const int width = grid.Width();
    const std::uint32_t visibleMask = (1u << grid.VisibleHeight()) - 1u;

    int total = 0;
    for (int x = 0; x < width; ++x) {
        total += ColumnHeight(grid, visibleMask, x);
    }
    return total;
}

int ComputeHoleDepth(const OccupancyGrid& grid) {
    // ColBits()はバッファ域も含む全高分のビットを持つため、見えている範囲だけに絞る。
    const std::uint32_t visibleMask = (1u << grid.VisibleHeight()) - 1u;

    int total = 0;
    for (int x = 0; x < grid.Width(); ++x) {
        std::uint32_t col = grid.ColBits(x) & visibleMask;
        if (col == 0) continue;  // 何も埋まっていない列に穴はない
        int highestBit = 31 - __builtin_clz(col);
        std::uint32_t belowMask = (1u << highestBit) - 1u;
        std::uint32_t holesMask = belowMask & ~col;  // 最上位ビットより下にある穴のビット集合

        std::uint32_t v = holesMask;
        while (v != 0) {
            int y = __builtin_ctz(v);
            total += __builtin_popcount(col >> (y + 1));  // yより上にある埋まったセルの数
            v &= v - 1;  // 最下位の1ビットをクリア
        }
    }
    return total;
}

int CountCumulativeWells(const OccupancyGrid& grid) {
    const int width = grid.Width();
    const int visibleHeight = grid.VisibleHeight();
    assert(visibleHeight < 32 && "candidateビットマスクがuint32_tに収まらなくなる");
    const std::uint32_t visibleMask = (visibleHeight >= 32) ? 0xFFFFFFFFu : ((1u << visibleHeight) - 1u);

    int cumulative = 0;
    for (int x = 0; x < width; ++x) {
        // 「自列が空・両隣(壁は埋扱い)が埋まっている」行だけを1にしたビット列。
        std::uint32_t leftFilled = (x == 0) ? visibleMask : grid.ColBits(x - 1);
        std::uint32_t rightFilled = (x == width - 1) ? visibleMask : grid.ColBits(x + 1);
        std::uint32_t candidate = (~grid.ColBits(x)) & leftFilled & rightFilled & visibleMask;

        // 元のループはdepthが連続する1の区間ごとに1,2,...,Lと増えて合計L(L+1)/2を足すのと同じなので、
        // 連続する1の区間(長さL)を見つけるたびにL(L+1)/2を加算する(区間の走査順は結果に影響しない)。
        // candidateはvisibleHeight<32ビットしか使わないため、v==0xFFFFFFFFにはなり得ず、
        // ループ中のctz呼び出しはすべて安全。
        std::uint32_t v = candidate;
        while (v != 0) {
            v >>= __builtin_ctz(v);
            int len = __builtin_ctz(~v);
            cumulative += len * (len + 1) / 2;
            v >>= len;
        }
    }
    return cumulative;
}

}  // namespace fb::ai::common
