#include "ai/beam/beam_search_player.hpp"

#include <algorithm>
#include <optional>

#include "ai/beam/evaluator.hpp"
#include "ai/common/executable_plan.hpp"
#include "ai/common/occupancy_grid.hpp"
#include "ai/common/reachable_search.hpp"
#include "ai/common/rotation_cache.hpp"
#include "ai/common/spawn_state.hpp"

namespace fb::ai::beam {

namespace {

struct BeamNode {
    common::OccupancyGrid board;
    double score = 0.0;
    std::vector<PlayerMove> rootPlan;  // 探索の起点で実際に実行する操作列(Holdを含む)。空 = まだ未決定。
    const core::Mino* holdMino = nullptr;
    int nextPieceIndex = 0;
};

void ExpandOption(const BeamNode& node, const core::Mino& pieceToPlace, bool usedHold,
                  const core::Mino* newHoldMino, int newNextPieceIndex, const Weights& weights,
                  std::vector<BeamNode>& out) {
    std::optional<common::SpawnState> spawn = common::ComputeSpawnState(node.board, pieceToPlace);
    if (!spawn.has_value()) return;  // このピースはこの盤面には出現できない(手詰まり)

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
            // 既にこの系統の起点の操作列は決定済み。そのまま引き継ぐ。
            child.rootPlan = node.rootPlan;
        } else {
            // この系統の起点の操作列を今ここで決定する。
            if (usedHold) child.rootPlan.push_back(PlayerMove::Hold);
            std::vector<PlayerMove> plan = common::ToExecutablePlan(placement.path);
            child.rootPlan.insert(child.rootPlan.end(), plan.begin(), plan.end());
        }

        out.push_back(std::move(child));
    }
}

// 1ラウンド分の展開作業の単位(ノード×パターン)。並列化の粒度として使う。
struct ExpandTask {
    const BeamNode* node = nullptr;
    bool isCarryOver = false;  // true: 展開せずnodeをそのまま持ち越すだけ
    const core::Mino* pieceToPlace = nullptr;
    bool usedHold = false;
    const core::Mino* newHoldMino = nullptr;
    int newNextPieceIndex = 0;
};

std::vector<ExpandTask> BuildExpandTasks(const std::vector<BeamNode>& beam,
                                          const std::vector<const core::Mino*>& sequence, int sequenceSize) {
    std::vector<ExpandTask> tasks;
    for (const BeamNode& node : beam) {
        if (node.nextPieceIndex >= sequenceSize) {
            tasks.push_back(ExpandTask{&node, /*isCarryOver=*/true});
            continue;
        }

        // Hold しないとき
        tasks.push_back(ExpandTask{&node, false, sequence[node.nextPieceIndex], false, node.holdMino, node.nextPieceIndex + 1});

        // Hold するとき(初回以外)
        if (node.holdMino != nullptr) {
            tasks.push_back(
                ExpandTask{&node, false, node.holdMino, true, sequence[node.nextPieceIndex], node.nextPieceIndex + 1});
        }

        // Hold するとき(初回)
        if (node.holdMino == nullptr && node.nextPieceIndex + 1 < sequenceSize) {
            tasks.push_back(ExpandTask{&node, false, sequence[node.nextPieceIndex + 1], true, sequence[node.nextPieceIndex],
                                        node.nextPieceIndex + 2});
        }
    }
    return tasks;
}

struct CandidateRef {
    double score = 0.0;
    int taskIdx = 0;
    int resultIdx = 0;
};

// sequence = [現在のピース, 次のsearchDepth個](Next表示の範囲内、合計searchDepth+1個)。
std::vector<const core::Mino*> BuildSequence(const core::Game& game, int searchDepth) {
    std::vector<const core::Mino*> sequence;
    sequence.push_back(game.GetActivePiece().MinoPtr());
    for (const core::Mino* m : game.PeekNext(searchDepth)) {
        sequence.push_back(m);
    }
    return sequence;
}

// perTaskResultsの全候補から、スコア上位beamWidth件を選んで次のビームにする(候補は移動される)。
// 候補が1つもなければ空のビームを返す。
std::vector<BeamNode> SelectNextBeam(std::vector<std::vector<BeamNode>>& perTaskResults, int beamWidth) {
    std::vector<CandidateRef> refs;
    std::size_t totalCandidates = 0;
    for (const auto& result : perTaskResults) totalCandidates += result.size();
    refs.reserve(totalCandidates);
    for (int i = 0; i < static_cast<int>(perTaskResults.size()); ++i) {
        const auto& result = perTaskResults[static_cast<std::size_t>(i)];
        for (int j = 0; j < static_cast<int>(result.size()); ++j) {
            refs.push_back(CandidateRef{result[static_cast<std::size_t>(j)].score, i, j});
        }
    }
    if (refs.empty()) return {};

    // 上位beam-width件だけ分かればいいので、全件ソートせずpartial_sortで済ませる。
    std::size_t keep = std::min(refs.size(), static_cast<std::size_t>(beamWidth));
    std::partial_sort(refs.begin(), refs.begin() + static_cast<std::ptrdiff_t>(keep), refs.end(),
                      [](const CandidateRef& a, const CandidateRef& b) {
                          if (a.score != b.score) return a.score > b.score;
                          if (a.taskIdx != b.taskIdx) return a.taskIdx < b.taskIdx;
                          return a.resultIdx < b.resultIdx;
                      });

    std::vector<BeamNode> nextBeam;
    nextBeam.reserve(keep);
    for (std::size_t k = 0; k < keep; ++k) {
        const CandidateRef& ref = refs[k];
        nextBeam.push_back(std::move(perTaskResults[static_cast<std::size_t>(ref.taskIdx)]
                                                    [static_cast<std::size_t>(ref.resultIdx)]));
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

    std::vector<const core::Mino*> minosToWarm = sequence;
    if (root.holdMino != nullptr) minosToWarm.push_back(root.holdMino);
    common::WarmUpRotationCaches(std::move(minosToWarm));

    std::vector<BeamNode> beam;
    beam.push_back(std::move(root));

    // sequenceを全ノードが読み切る(nextPieceIndexがsequenceSize以上になる)まで繰り返す。
    // 最初のホールドは1手でsequenceを2個進めるため、ノードごとにnextPieceIndexが異なりうる
    // (=ステップ数が固定にならない)。
    //
    // #pragma omp parallelでスレッドチームをループ全体で1回だけ作り、ラウンドごとの
    // チーム編成・解体のオーバーヘッドをなくす。逐次部分はsingleで1スレッドだけ実行し、
    // (暗黙のバリアにより)全スレッドがbeam/tasks/perTaskResultsなどを同じタイミングで
    // 同じ内容として見られるようにしている。これらの変数はparallelの外側(ここ)で
    // 宣言しているのでスレッド間で共有される。
    bool anyPending = false;
    bool hasCandidates = false;
    std::vector<ExpandTask> tasks;
    std::vector<std::vector<BeamNode>> perTaskResults;

#pragma omp parallel default(shared)
    {
        while (true) {
#pragma omp single
            {
                anyPending = false;
                for (const auto& node : beam) {
                    if (node.nextPieceIndex < sequenceSize) {
                        anyPending = true;
                        break;
                    }
                }
                if (anyPending) {
                    tasks = BuildExpandTasks(beam, sequence, sequenceSize);
                    perTaskResults.assign(tasks.size(), {});
                }
            }  // 全スレッドがここでanyPending/tasks/perTaskResultsを同じ値で見る

            if (!anyPending) break;

#pragma omp for schedule(dynamic)
            for (int i = 0; i < static_cast<int>(tasks.size()); ++i) {
                const ExpandTask& task = tasks[i];
                if (task.isCarryOver) {
                    perTaskResults[static_cast<std::size_t>(i)].push_back(*task.node);  // そのまま持ち越す。
                    continue;
                }
                ExpandOption(*task.node, *task.pieceToPlace, task.usedHold, task.newHoldMino, task.newNextPieceIndex,
                             weights_, perTaskResults[static_cast<std::size_t>(i)]);
            }  // 全スレッドのExpandOptionが完了してから先に進む

#pragma omp single
            {
                std::vector<BeamNode> nextBeam = SelectNextBeam(perTaskResults, beamWidth_);
                hasCandidates = !nextBeam.empty();
                if (hasCandidates) beam = std::move(nextBeam);
            }

            if (!hasCandidates) break;  // どの候補からも置けなくなった(それ以上先は読めない)
        }
    }

    if (beam.empty()) return {};
    return beam.front().rootPlan;
}

}  // namespace fb::ai::beam
