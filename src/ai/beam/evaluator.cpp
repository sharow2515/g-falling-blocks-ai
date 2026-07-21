#include "ai/beam/evaluator.hpp"

#include <algorithm>
#include <limits>

namespace fb::ai::beam {

namespace {

int CountHoles(const core::Board& board) {
    int holes = 0;
    for (int x = 0; x < board.Width(); ++x) {
        bool seenFilled = false;
        for (int y = board.Height() - 1; y >= 0; --y) {
            bool occupied = board.CellColor({x, y}).has_value();
            if (occupied) {
                seenFilled = true;
            } else if (seenFilled) {
                ++holes;
            }
        }
    }
    return holes;
}

int CountRowTransitions(const core::Board& board) {
    int transitions = 0;
    for (int y = 0; y < board.Height(); ++y) {
        bool prevFilled = true;  // 左の壁は「埋」扱い
        for (int x = 0; x <= board.Width(); ++x) {
            bool filled = (x == board.Width()) ? true : board.CellColor({x, y}).has_value();
            if (filled != prevFilled) ++transitions;
            prevFilled = filled;
        }
    }
    return transitions;
}

int CountColumnTransitions(const core::Board& board) {
    int transitions = 0;
    for (int x = 0; x < board.Width(); ++x) {
        bool prevFilled = true;  // 床は「埋」扱い
        for (int y = 0; y < board.Height(); ++y) {
            bool filled = board.CellColor({x, y}).has_value();
            if (filled != prevFilled) ++transitions;
            prevFilled = filled;
        }
    }
    return transitions;
}

int CountCumulativeWells(const core::Board& board) {
    int cumulative = 0;
    for (int x = 0; x < board.Width(); ++x) {
        int depth = 0;
        for (int y = board.Height() - 1; y >= 0; --y) {
            bool filled = board.CellColor({x, y}).has_value();
            bool leftFilled = (x == 0) ? true : board.CellColor({x - 1, y}).has_value();
            bool rightFilled = (x == board.Width() - 1) ? true : board.CellColor({x + 1, y}).has_value();

            if (!filled && leftFilled && rightFilled) {
                ++depth;
                cumulative += depth;
            } else {
                depth = 0;
            }
        }
    }
    return cumulative;
}

}  // namespace

double Evaluate(const Features& f) {
    return -1.0 * f.landingHeight + 1.0 * f.erodedPieceCells - 1.0 * f.rowTransitions -
           1.0 * f.columnTransitions - 4.0 * f.holes - 1.0 * f.cumulativeWells;
}

Features ComputeFeaturesFromClearedBoard(const core::Board& clearedBoard, const std::vector<core::Vec2i>& cells,
                                          const std::vector<int>& clearedRows) {
    Features f;

    int minRow = std::numeric_limits<int>::max();
    int maxRow = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        minRow = std::min(minRow, c.y);
        maxRow = std::max(maxRow, c.y);
    }
    f.landingHeight = (minRow + maxRow) / 2.0;

    int erodedCellCount = 0;
    for (int row : clearedRows) {
        for (const auto& c : cells) {
            if (c.y == row) ++erodedCellCount;
        }
    }
    f.erodedPieceCells = static_cast<double>(clearedRows.size()) * static_cast<double>(erodedCellCount);

    f.holes = CountHoles(clearedBoard);
    f.rowTransitions = CountRowTransitions(clearedBoard);
    f.columnTransitions = CountColumnTransitions(clearedBoard);
    f.cumulativeWells = CountCumulativeWells(clearedBoard);

    return f;
}

Features ComputeFeatures(core::Board board, const std::vector<core::Vec2i>& cells, core::Color color) {
    board.PlacePiece(cells, core::Vec2i{0, 0}, color);
    std::vector<int> clearedRows;
    board.ClearFullLines(clearedRows);
    return ComputeFeaturesFromClearedBoard(board, cells, clearedRows);
}

double ScorePlacement(const core::Board& board, const std::vector<core::Vec2i>& cells, core::Color color) {
    return Evaluate(ComputeFeatures(board, cells, color));
}

}  // namespace fb::ai::beam
