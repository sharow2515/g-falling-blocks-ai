#include "ai/beam_unique/beam_search_player.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <unordered_map>

#include "ai/beam_unique/evaluator.hpp"
#include "ai/common/occupancy_grid.hpp"
#include "ai/common/path_conversion.hpp"
#include "ai/common/reachable_search.hpp"

namespace fb::ai::beam_unique {

namespace {

struct SpawnState {
    int rotation;
    core::Vec2i position;
};

// core::ActivePiece::Initialize と同じスポーン位置の式(回転0固定、衝突時2回リトライ)。
// まだ実際にはスポーンしていない未来のピースについて、「もしこの盤面に今出現したら
// どこから始まるか」を計算するためのもの。core::ActivePiece自体には手を入れず、
// この用途専用にここでだけ複製している。
std::optional<SpawnState> ComputeSpawnState(const common::OccupancyGrid& board, const core::Mino& mino) {
    const std::vector<core::Vec2i>& cells = mino.GetCells(0);

    int maxCellY = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        if (c.y > maxCellY) maxCellY = c.y;
    }

    int spawnX = board.Width() / 2 - mino.gridSize / 2;
    int spawnY = board.VisibleHeight() - maxCellY - 1;
    core::Vec2i pos{spawnX, spawnY};

    if (!board.IsValidPosition(cells, pos)) {
        pos.y += 1;
        if (!board.IsValidPosition(cells, pos)) {
            pos.y += 1;
            if (!board.IsValidPosition(cells, pos)) {
                return std::nullopt;
            }
        }
    }

    return SpawnState{0, pos};
}

// FNV-1a(64bit)で盤面をハッシュ化する。あくまで重複候補を素早く絞り込むための
// 事前フィルタであり、衝突しても(別々のハッシュ値の候補を誤って同一視することは
// ないが、逆に)ハッシュが一致した候補同士は必ずBoardsEqualで実際の中身を検証する。
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

// ビームサーチ中の1候補。
// Holdにより、ノードごとに「sequence中どこまで消費し終えたか(seqIndex)」が
// 異なりうる(初回Holdは1手で2個消費するため)。
struct BeamNode {
    common::OccupancyGrid board;
    double score = 0.0;
    std::vector<PlayerMove> firstMove;  // 探索起点(現在のピース)に対する操作列。空 = まだ未決定。
    const core::Mino* holdMino = nullptr;
    int seqIndex = 0;
};

// nodeの盤面にpieceToPlaceを置く全候補を展開し、outに追加する。
// usedHold: この手でHoldコマンドを使ったか(firstMove構築のため)。
// newHoldMino/newSeqIndex: この手を打った後のHold内容・sequence消費位置。
void ExpandOption(const BeamNode& node, const core::Mino& pieceToPlace, bool usedHold,
                  const core::Mino* newHoldMino, int newSeqIndex, const Weights& weights,
                  std::vector<BeamNode>& out) {
    std::optional<SpawnState> spawn = ComputeSpawnState(node.board, pieceToPlace);
    if (!spawn.has_value()) return;  // このピースはこの盤面には出現できない(手詰まり)

    std::vector<common::ReachablePlacement> placements =
        common::SearchReachablePlacements(node.board, pieceToPlace, spawn->rotation, spawn->position);

    for (const auto& placement : placements) {
        common::OccupancyGrid newBoard = node.board;
        newBoard.PlacePiece(placement.cells, core::Vec2i{0, 0});
        std::vector<int> clearedRows;
        newBoard.ClearFullLines(clearedRows);

        // 合算しない: 今置き終えたばかりのこの盤面を評価した値でスコアを上書きする。
        double score = Evaluate(ComputeFeaturesFromClearedBoard(newBoard, placement.cells, clearedRows), weights);

        BeamNode child;
        child.board = std::move(newBoard);
        child.score = score;
        child.holdMino = newHoldMino;
        child.seqIndex = newSeqIndex;

        if (!node.firstMove.empty()) {
            // 既にこの系統の1手目は決定済み。そのまま引き継ぐ。
            child.firstMove = node.firstMove;
        } else {
            // この系統の1手目を今ここで決定する。
            if (usedHold) child.firstMove.push_back(PlayerMove::Hold);
            std::vector<PlayerMove> converted = common::ConvertPath(placement.path);
            child.firstMove.insert(child.firstMove.end(), converted.begin(), converted.end());
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
    int newSeqIndex = 0;
};

// beamの各ノードに対して、このラウンドで試すべきタスク(展開パターン)を列挙する。
// 元の逐次ループと全く同じ順序で列挙するため、ここで作ったタスク列を
// タスク番号順に結合すれば、並列実行してもnextBeamの並びは逐次版と完全に一致する。
std::vector<ExpandTask> BuildExpandTasks(const std::vector<BeamNode>& beam,
                                          const std::vector<const core::Mino*>& sequence, int sequenceSize) {
    std::vector<ExpandTask> tasks;
    for (const BeamNode& node : beam) {
        if (node.seqIndex >= sequenceSize) {
            tasks.push_back(ExpandTask{&node, /*isCarryOver=*/true});
            continue;
        }

        // (a) sequence[seqIndex] をそのまま置く(Holdしない)。
        tasks.push_back(ExpandTask{&node, false, sequence[node.seqIndex], false, node.holdMino, node.seqIndex + 1});

        // (b) スワップHold: Hold中の駒を出して置き、sequence[seqIndex]をHoldに預ける。
        if (node.holdMino != nullptr) {
            tasks.push_back(
                ExpandTask{&node, false, node.holdMino, true, sequence[node.seqIndex], node.seqIndex + 1});
        }

        // (c) 初回Hold: Holdが空。sequence[seqIndex]をHoldに預け、次のsequence[seqIndex+1]を置く。
        if (node.holdMino == nullptr && node.seqIndex + 1 < sequenceSize) {
            tasks.push_back(ExpandTask{&node, false, sequence[node.seqIndex + 1], true, sequence[node.seqIndex],
                                        node.seqIndex + 2});
        }
    }
    return tasks;
}

struct CandidateRef {
    double score = 0.0;
    std::uint64_t boardHash = 0;
    int taskIdx = 0;
    int resultIdx = 0;
};

}  // namespace

BeamSearchPlayer::BeamSearchPlayer(int beamWidth, int searchDepth, Weights weights)
    : beamWidth_(beamWidth), searchDepth_(searchDepth), weights_(weights) {
    if (beamWidth < 1) {
        throw std::invalid_argument("BeamSearchPlayer: beamWidthは1以上である必要があります");
    }
    if (searchDepth < 0 || searchDepth > kMaxSearchDepth) {
        throw std::invalid_argument("BeamSearchPlayer: searchDepthは0から" +
                                     std::to_string(kMaxSearchDepth) +
                                     "の範囲である必要があります(Next表示を超える先読みは不可)");
    }
}

std::vector<PlayerMove> BeamSearchPlayer::Plan(const core::Game& game) {
    if (game.IsGameOver() || game.IsPaused()) return {};
    if (!game.GetActivePiece().IsActive()) return {};

    const core::Mino* currentMino = game.GetActivePiece().MinoPtr();
    if (currentMino == nullptr) return {};

    // sequence = [現在のピース, 次のsearchDepth_個](Next表示の範囲内、合計searchDepth_+1個)。
    std::vector<const core::Mino*> sequence;
    sequence.push_back(currentMino);
    for (const core::Mino* m : game.PeekNext(searchDepth_)) {
        sequence.push_back(m);
    }
    const int sequenceSize = static_cast<int>(sequence.size());

    std::vector<BeamNode> beam;
    BeamNode root;
    root.board = common::OccupancyGrid(game.GetBoard());
    root.holdMino = game.GetHoldMino();
    root.seqIndex = 0;

    std::vector<const core::Mino*> minosToWarm = sequence;
    if (root.holdMino != nullptr) minosToWarm.push_back(root.holdMino);
    std::sort(minosToWarm.begin(), minosToWarm.end());
    minosToWarm.erase(std::unique(minosToWarm.begin(), minosToWarm.end()), minosToWarm.end());
    for (const core::Mino* m : minosToWarm) {
        for (int r = 0; r < 4; ++r) static_cast<void>(m->GetCells(r));
    }

    beam.push_back(std::move(root));

    // sequenceを全ノードが読み切るまで繰り返す。初回Hold(Hold空の状態からHoldする)は
    // 1手でsequenceを2個消費するため、ノードごとに消費済み位置(seqIndex)が異なりうる
    // (=ステップ数が固定にならない)。
    //
    // #pragma omp parallelでスレッドチームをループ全体で1回だけ作り、ラウンドごとの
    // チーム編成・解体のオーバーヘッドをなくす。逐次部分はsingleで1スレッドだけ実行し、
    // (暗黙のバリアにより)全スレッドがbeam/tasks/perTaskResults/refsを同じタイミングで
    // 同じ内容として見られるようにしている。これらの変数はparallelの外側(ここ)で
    // 宣言しているのでスレッド間で共有される。
    bool anyPending = false;
    std::vector<ExpandTask> tasks;
    std::vector<std::vector<BeamNode>> perTaskResults;
    std::vector<CandidateRef> refs;
    std::vector<BeamNode> nextBeam;

#pragma omp parallel default(shared)
    {
        while (true) {
#pragma omp single
            {
                anyPending = false;
                for (const auto& node : beam) {
                    if (node.seqIndex < sequenceSize) {
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
                ExpandOption(*task.node, *task.pieceToPlace, task.usedHold, task.newHoldMino, task.newSeqIndex,
                             weights_, perTaskResults[static_cast<std::size_t>(i)]);
            }  // 全スレッドのExpandOptionが完了してから先に進む

#pragma omp single
            {
                refs.clear();
                std::size_t totalCandidates = 0;
                for (const auto& result : perTaskResults) totalCandidates += result.size();
                refs.reserve(totalCandidates);
                for (int i = 0; i < static_cast<int>(perTaskResults.size()); ++i) {
                    const auto& result = perTaskResults[static_cast<std::size_t>(i)];
                    for (int j = 0; j < static_cast<int>(result.size()); ++j) {
                        const BeamNode& resultNode = result[static_cast<std::size_t>(j)];
                        refs.push_back(CandidateRef{resultNode.score, HashBoard(resultNode.board), i, j});
                    }
                }

                if (!refs.empty()) {
                    // スコア降順に全件並べ、上位から順に「(盤面, Hold中身, seqIndex)が
                    // まだ採用されていない」候補だけをbeam-width件になるまで採用する
                    // (=同一状態への重複投票を1つにまとめ、浮いた枠を次点の異なる状態に回す)。
                    // ハッシュはあくまで事前フィルタで、同じハッシュの候補同士は実際の
                    // 盤面・Hold・seqIndexを比較して初めて「同一」と判定する
                    // (ハッシュ衝突があっても誤って別状態を捨てることはない)。
                    std::sort(refs.begin(), refs.end(), [](const CandidateRef& a, const CandidateRef& b) {
                        if (a.score != b.score) return a.score > b.score;
                        if (a.taskIdx != b.taskIdx) return a.taskIdx < b.taskIdx;
                        return a.resultIdx < b.resultIdx;
                    });

                    std::size_t keep = std::min(refs.size(), static_cast<std::size_t>(beamWidth_));
                    std::unordered_map<std::uint64_t, std::vector<std::size_t>> keptByHash;
                    nextBeam.clear();
                    nextBeam.reserve(keep);

                    for (const CandidateRef& ref : refs) {
                        if (nextBeam.size() >= keep) break;

                        BeamNode& candidate = perTaskResults[static_cast<std::size_t>(ref.taskIdx)]
                                                             [static_cast<std::size_t>(ref.resultIdx)];

                        bool isDuplicate = false;
                        auto it = keptByHash.find(ref.boardHash);
                        if (it != keptByHash.end()) {
                            for (std::size_t keptIdx : it->second) {
                                const BeamNode& kept = nextBeam[keptIdx];
                                if (kept.holdMino == candidate.holdMino && kept.seqIndex == candidate.seqIndex &&
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

                    beam = std::move(nextBeam);
                }
            }

            if (refs.empty()) break;  // どの候補からも置けなくなった(それ以上先は読めない)
        }
    }

    if (beam.empty()) return {};
    return beam.front().firstMove;
}

}  // namespace fb::ai::beam_unique
