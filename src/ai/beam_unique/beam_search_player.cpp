#include "ai/beam_unique/beam_search_player.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "ai/beam_unique/evaluator.hpp"
#include "ai/common/executable_plan.hpp"
#include "ai/common/occupancy_grid.hpp"
#include "ai/common/reachable_search.hpp"
#include "ai/common/spawn_state.hpp"

namespace fb::ai::beam_unique {

namespace {

// FNV-1a(64bit)で盤面をハッシュ化する。同じ盤面の候補を素早く見つけるための事前フィルタで、
// ハッシュが一致した候補同士は、必ずBoardsEqualで実際の中身を比べて同一か判定する。
std::uint64_t HashBoard(const common::OccupancyGrid& board) {
    std::uint64_t h = 14695981039346656037ull;  // FNV offset basis
    for (int y = 0; y < board.TotalHeight(); ++y) {
        std::uint32_t row = board.RowBits(y);
        for (int b = 0; b < 4; ++b) {
            h ^= static_cast<std::uint8_t>((row >> (b * 8)) & 0xFF);
            h *= 1099511628211ull;  // FNV prime
        }
    }
    return h;
}

bool BoardsEqual(const common::OccupancyGrid& a, const common::OccupancyGrid& b) {
    if (a.Width() != b.Width() || a.TotalHeight() != b.TotalHeight()) return false;
    for (int y = 0; y < a.TotalHeight(); ++y) {
        if (a.RowBits(y) != b.RowBits(y)) return false;
    }
    return true;
}

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
    std::uint64_t boardHash = 0;
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

std::vector<BeamNode> SelectNextBeam(std::vector<BeamNode>& candidates, int beamWidth) {
    if (candidates.empty()) return {};

    std::vector<CandidateRef> refs;
    refs.reserve(candidates.size());
    for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
        const BeamNode& candidate = candidates[static_cast<std::size_t>(i)];
        refs.push_back(CandidateRef{candidate.score, HashBoard(candidate.board), i});
    }

    std::sort(refs.begin(), refs.end(), [](const CandidateRef& a, const CandidateRef& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.index < b.index;
    });

    const std::size_t keep = std::min(refs.size(), static_cast<std::size_t>(beamWidth));
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> keptByHash;  // ハッシュ -> nextBeam内の位置
    std::vector<BeamNode> nextBeam;
    nextBeam.reserve(keep);

    for (const CandidateRef& ref : refs) {
        if (nextBeam.size() >= keep) break;

        BeamNode& candidate = candidates[static_cast<std::size_t>(ref.index)];

        bool isDuplicate = false;
        auto it = keptByHash.find(ref.boardHash);
        if (it != keptByHash.end()) {
            for (std::size_t keptIndex : it->second) {
                const BeamNode& kept = nextBeam[keptIndex];
                if (kept.holdMino == candidate.holdMino && kept.nextPieceIndex == candidate.nextPieceIndex &&
                    BoardsEqual(kept.board, candidate.board)) {
                    isDuplicate = true;
                    break;
                }
            }
        }
        if (isDuplicate) continue;

        keptByHash[ref.boardHash].push_back(nextBeam.size());
        nextBeam.push_back(std::move(candidate));
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

}  // namespace fb::ai::beam_unique
