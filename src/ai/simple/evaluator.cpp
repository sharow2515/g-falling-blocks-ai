#include "ai/simple/evaluator.hpp"

#include "ai/common/board_features.hpp"

namespace fb::ai {

double Evaluate(const SimpleFeatures& f) {
    return -1.0 * f.landingHeight + 1.0 * f.erodedPieceCells - 1.0 * f.rowTransitions -
           1.0 * f.columnTransitions - 4.0 * f.holes - 1.0 * f.cumulativeWells;
}

SimpleFeatures ComputeFeatures(core::Board board, const Placement& placement, core::Color color) {
    SimpleFeatures f;
    f.landingHeight = common::ComputeLandingHeight(placement.cells);

    board.PlacePiece(placement.cells, core::Vec2i{0, 0}, color);

    std::vector<int> clearedRows;
    board.ClearFullLines(clearedRows);

    f.erodedPieceCells = common::ComputeErodedPieceCells(placement.cells, clearedRows);
    f.holes = common::CountHoles(board);
    f.rowTransitions = common::CountRowTransitions(board);
    f.columnTransitions = common::CountColumnTransitions(board);
    f.cumulativeWells = common::CountCumulativeWells(board);

    return f;
}

double ScorePlacement(const core::Board& board, const Placement& placement, core::Color color) {
    return Evaluate(ComputeFeatures(board, placement, color));
}

}  // namespace fb::ai
