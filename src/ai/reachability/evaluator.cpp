#include "ai/reachability/evaluator.hpp"

#include "ai/common/board_features.hpp"

namespace fb::ai::reachability {

double Evaluate(const Features& f) {
    return -1.0 * f.landingHeight + 1.0 * f.erodedPieceCells - 1.0 * f.rowTransitions -
           1.0 * f.columnTransitions - 4.0 * f.holes - 1.0 * f.cumulativeWells;
}

Features ComputeFeatures(core::Board board, const std::vector<core::Vec2i>& cells, core::Color color) {
    Features f;
    f.landingHeight = common::ComputeLandingHeight(cells);

    board.PlacePiece(cells, core::Vec2i{0, 0}, color);

    std::vector<int> clearedRows;
    board.ClearFullLines(clearedRows);

    f.erodedPieceCells = common::ComputeErodedPieceCells(cells, clearedRows);
    f.holes = common::CountHoles(board);
    f.rowTransitions = common::CountRowTransitions(board);
    f.columnTransitions = common::CountColumnTransitions(board);
    f.cumulativeWells = common::CountCumulativeWells(board);

    return f;
}

double ScorePlacement(const core::Board& board, const std::vector<core::Vec2i>& cells, core::Color color) {
    return Evaluate(ComputeFeatures(board, cells, color));
}

}  // namespace fb::ai::reachability
