#include "ai/beam/beam_search_player.hpp"

#include <algorithm>
#include <optional>

#include "ai/beam/evaluator.hpp"
#include "ai/common/executable_plan.hpp"
#include "ai/common/occupancy_grid.hpp"
#include "ai/common/reachable_search.hpp"
#include "ai/common/spawn_state.hpp"

namespace fb::ai::beam {

namespace {

struct BeamNode {
    common::OccupancyGrid board;
    double score = 0.0;
    std::vector<PlayerMove> rootPlan;
    const core::Mino* holdMino = nullptr;
    int nextPieceIndex = 0;
};

void ExpandOption(const BeamNode& node, const core::Mino& pieceToPlace, bool usedHold,
                  const core::Mino* newHoldMino, int newNextPieceIndex, const Weights& weights,
                  std::vector<BeamNode>& out) {
    std::optional<common::SpawnState> spawn = common::ComputeSpawnState(node.board, pieceToPlace);
    if (!spawn.has_value()) return; // ピースが出現できない

    std::vector<common::ReachablePlacement> placements =
        common::SearchReachablePlacements(node.board, pieceToPlace, spawn->rotation, spawn->position);

    for (const auto& placement : placements) {
        common::OccupancyGrid newBoard = node.board;
        newBoard.PlacePiece(placement.cells, core::Vec2i{0, 0});
        std::vector<int> clearedRows;
        newBoard.ClearFullLines(clearedRows);

        double score = Evaluate(ComputeFeaturesFromClearedBoard(newBoard, placement.cells, clearedRows), weights);

        BeamNode child;
        child.board = std::move(newBoard);
        child.score = score;
        child.holdMino = newHoldMino;
        child.nextPieceIndex = newNextPieceIndex;

        if (!node.rootPlan.empty()) {
            // 操作列は既に決まっている
            child.rootPlan = node.rootPlan;
        } else {
            // 操作列を決める
            if (usedHold) child.rootPlan.push_back(PlayerMove::Hold);
            std::vector<PlayerMove> plan = common::ToExecutablePlan(placement.path);
            child.rootPlan.insert(child.rootPlan.end(), plan.begin(), plan.end());
        }

        out.push_back(std::move(child));
    }
}

void ExpandNode(const BeamNode& node, const std::vector<const core::Mino*>& sequence, const Weights& weights,
                std::vector<BeamNode>& candidates) {
    const int sequenceSize = static_cast<int>(sequence.size());

    if (node.nextPieceIndex >= sequenceSize) {
        candidates.push_back(node);
        return;
    }

    // Hold しないとき
    ExpandOption(node, *sequence[node.nextPieceIndex], false, node.holdMino, node.nextPieceIndex + 1, weights,
                 candidates);

    // Hold するとき(初回以外)
    if (node.holdMino != nullptr) {
        ExpandOption(node, *node.holdMino, true, sequence[node.nextPieceIndex], node.nextPieceIndex + 1, weights,
                     candidates);
    }

    // Hold するとき(初回)
    if (node.holdMino == nullptr && node.nextPieceIndex + 1 < sequenceSize) {
        ExpandOption(node, *sequence[node.nextPieceIndex + 1], true, sequence[node.nextPieceIndex],
                     node.nextPieceIndex + 2, weights, candidates);
    }
}

struct CandidateRef {
    double score = 0.0;
    int index = 0;  // candidates内の位置(同点のときは小さい方を優先する)
};

// 落ちてくるブロックの順番を取得
std::vector<const core::Mino*> BuildSequence(const core::Game& game, int searchDepth) {
    std::vector<const core::Mino*> sequence;
    sequence.push_back(game.GetActivePiece().MinoPtr());
    for (const core::Mino* m : game.PeekNext(searchDepth)) {
        sequence.push_back(m);
    }
    return sequence;
}

// candidatesから、スコア上位beamWidth件を選んで次のビームにする(候補は移動される)。
// 候補が1つもなければ空のビームを返す。
std::vector<BeamNode> SelectNextBeam(std::vector<BeamNode>& candidates, int beamWidth) {
    if (candidates.empty()) return {};

    std::vector<CandidateRef> refs;
    refs.reserve(candidates.size());
    for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
        refs.push_back(CandidateRef{candidates[static_cast<std::size_t>(i)].score, i});
    }

    std::size_t keep = std::min(refs.size(), static_cast<std::size_t>(beamWidth));
    std::partial_sort(refs.begin(), refs.begin() + static_cast<std::ptrdiff_t>(keep), refs.end(),
                      [](const CandidateRef& a, const CandidateRef& b) {
                          if (a.score != b.score) return a.score > b.score;
                          return a.index < b.index;
                      });

    std::vector<BeamNode> nextBeam;
    nextBeam.reserve(keep);
    for (std::size_t k = 0; k < keep; ++k) {
        nextBeam.push_back(std::move(candidates[static_cast<std::size_t>(refs[k].index)]));
    }
    return nextBeam;
}

}  // namespace

BeamSearchPlayer::BeamSearchPlayer(int beamWidth, int searchDepth, Weights weights)
    : beamWidth_(beamWidth), searchDepth_(searchDepth), weights_(weights) {
    if (beamWidth < 1) {
        throw std::invalid_argument("BeamSearchPlayer: beamWidthは1以上である必要があります");
    }
    if (searchDepth < 0 || searchDepth > kMaxSearchDepth) {
        throw std::invalid_argument("BeamSearchPlayer: searchDepthは0から" +
                                     std::to_string(kMaxSearchDepth) +
                                     "の範囲である必要があります");
    }
}

std::vector<PlayerMove> BeamSearchPlayer::Plan(const core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const std::vector<const core::Mino*> sequence = BuildSequence(game, searchDepth_);
    const int sequenceSize = static_cast<int>(sequence.size());

    BeamNode root;
    root.board = common::OccupancyGrid(game.GetBoard());
    root.holdMino = game.GetHoldMino();
    root.nextPieceIndex = 0;

    std::vector<BeamNode> beam;
    beam.push_back(std::move(root));

    constexpr std::size_t kExpectedCandidatesPerNode = 128;
    std::vector<BeamNode> candidates;
    candidates.reserve(static_cast<std::size_t>(beamWidth_) * kExpectedCandidatesPerNode);
    while (true) {
        const bool anyPending = std::any_of(beam.begin(), beam.end(), [sequenceSize](const BeamNode& node) {
            return node.nextPieceIndex < sequenceSize;
        });
        if (!anyPending) break;

        candidates.clear();
        for (const BeamNode& node : beam) {
            ExpandNode(node, sequence, weights_, candidates);
        }

        std::vector<BeamNode> nextBeam = SelectNextBeam(candidates, beamWidth_);
        if (nextBeam.empty()) break; // どの候補からも置けなくなった(それ以上先は読めない)
        beam = std::move(nextBeam);
    }

    if (beam.empty()) return {};
    return beam.front().rootPlan;
}

}  // namespace fb::ai::beam
