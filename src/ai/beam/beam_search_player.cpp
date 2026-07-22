#include "ai/beam/beam_search_player.hpp"

#include <algorithm>
#include <limits>
#include <optional>

#include "ai/beam/evaluator.hpp"
#include "ai/common/move_execution.hpp"
#include "ai/common/path_conversion.hpp"
#include "ai/common/reachable_search.hpp"

namespace fb::ai::beam {

namespace {

struct SpawnState {
    int rotation;
    core::Vec2i position;
};

// core::ActivePiece::Initialize と同じスポーン位置の式(回転0固定、衝突時2回リトライ)。
// まだ実際にはスポーンしていない未来のピースについて、「もしこの盤面に今出現したら
// どこから始まるか」を計算するためのもの。core::ActivePiece自体には手を入れず、
// この用途専用にここでだけ複製している。
std::optional<SpawnState> ComputeSpawnState(const core::Board& board, const core::Mino& mino) {
    const std::vector<core::Vec2i>& cells = mino.GetCells(0);

    int maxCellY = std::numeric_limits<int>::min();
    for (const auto& c : cells) {
        if (c.y > maxCellY) maxCellY = c.y;
    }

    int spawnX = board.Width() / 2 - mino.gridSize / 2;
    int spawnY = board.Height() - maxCellY - 1;
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

// ビームサーチ中の1候補。
// Holdにより、ノードごとに「sequence中どこまで消費し終えたか(seqIndex)」が
// 異なりうる(初回Holdは1手で2個消費するため)。
struct BeamNode {
    core::Board board;
    double score = 0.0;
    std::vector<PlayerMove> firstMove;  // 探索起点(現在のピース)に対する操作列。空 = まだ未決定。
    const core::Mino* holdMino = nullptr;
    int seqIndex = 0;
};

// nodeの盤面にpieceToPlaceを置く全候補を展開し、outに追加する。
// usedHold: この手でHoldコマンドを使ったか(firstMove構築のため)。
// newHoldMino/newSeqIndex: この手を打った後のHold内容・sequence消費位置。
void ExpandOption(const BeamNode& node, const core::Mino& pieceToPlace, bool usedHold,
                  const core::Mino* newHoldMino, int newSeqIndex, std::vector<BeamNode>& out) {
    std::optional<SpawnState> spawn = ComputeSpawnState(node.board, pieceToPlace);
    if (!spawn.has_value()) return;  // このピースはこの盤面には出現できない(手詰まり)

    std::vector<common::ReachablePlacement> placements =
        common::SearchReachablePlacements(node.board, pieceToPlace, spawn->rotation, spawn->position);

    for (const auto& placement : placements) {
        core::Board newBoard = node.board;
        newBoard.PlacePiece(placement.cells, core::Vec2i{0, 0}, pieceToPlace.color);
        std::vector<int> clearedRows;
        newBoard.ClearFullLines(clearedRows);

        // 合算しない: 今置き終えたばかりのこの盤面を評価した値でスコアを上書きする。
        double score = Evaluate(ComputeFeaturesFromClearedBoard(newBoard, placement.cells, clearedRows));

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

}  // namespace

BeamSearchPlayer::BeamSearchPlayer(int beamWidth, int searchDepth)
    : beamWidth_(beamWidth), searchDepth_(searchDepth) {
    if (beamWidth < 1) {
        throw std::invalid_argument("BeamSearchPlayer: beamWidthは1以上である必要があります");
    }
    if (searchDepth < 0 || searchDepth > kMaxSearchDepth) {
        throw std::invalid_argument("BeamSearchPlayer: searchDepthは0から" +
                                     std::to_string(kMaxSearchDepth) +
                                     "の範囲である必要があります(Next表示を超える先読みは不可)");
    }
}

void BeamSearchPlayer::Update(core::Game& game) {
    int spawnId = game.SpawnEventId();
    if (spawnId == lastSeenSpawnEventId_) return;
    lastSeenSpawnEventId_ = spawnId;

    for (PlayerMove move : BuildPlan(game)) {
        common::ExecuteMove(game, move);
    }
}

std::vector<PlayerMove> BeamSearchPlayer::Plan(core::Game& game) { return BuildPlan(game); }

std::vector<PlayerMove> BeamSearchPlayer::BuildPlan(core::Game& game) {
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
    root.board = game.GetBoard();
    root.holdMino = game.GetHoldMino();
    root.seqIndex = 0;
    beam.push_back(std::move(root));

    // sequenceを全ノードが読み切るまで繰り返す。初回Hold(Hold空の状態からHoldする)は
    // 1手でsequenceを2個消費するため、ノードごとに消費済み位置(seqIndex)が異なりうる
    // (=ステップ数が固定にならない)。
    while (true) {
        bool anyPending = false;
        for (const auto& node : beam) {
            if (node.seqIndex < sequenceSize) {
                anyPending = true;
                break;
            }
        }
        if (!anyPending) break;

        std::vector<BeamNode> nextBeam;

        for (const BeamNode& node : beam) {
            if (node.seqIndex >= sequenceSize) {
                nextBeam.push_back(node);  // 既に読み切っている。そのまま持ち越す。
                continue;
            }

            // (a) sequence[seqIndex] をそのまま置く(Holdしない)。
            ExpandOption(node, *sequence[node.seqIndex], /*usedHold=*/false, node.holdMino, node.seqIndex + 1,
                         nextBeam);

            // (b) スワップHold: Hold中の駒を出して置き、sequence[seqIndex]をHoldに預ける。
            if (node.holdMino != nullptr) {
                ExpandOption(node, *node.holdMino, /*usedHold=*/true, sequence[node.seqIndex],
                             node.seqIndex + 1, nextBeam);
            }

            // (c) 初回Hold: Holdが空。sequence[seqIndex]をHoldに預け、次のsequence[seqIndex+1]を置く。
            if (node.holdMino == nullptr && node.seqIndex + 1 < sequenceSize) {
                ExpandOption(node, *sequence[node.seqIndex + 1], /*usedHold=*/true, sequence[node.seqIndex],
                             node.seqIndex + 2, nextBeam);
            }
        }

        if (nextBeam.empty()) break;  // どの候補からも置けなくなった(それ以上先は読めない)

        // 同点候補は先に見つかったものを優先するため安定ソートを使う
        std::stable_sort(nextBeam.begin(), nextBeam.end(),
                          [](const BeamNode& a, const BeamNode& b) { return a.score > b.score; });
        if (nextBeam.size() > static_cast<std::size_t>(beamWidth_)) {
            nextBeam.resize(beamWidth_);
        }
        beam = std::move(nextBeam);
    }

    if (beam.empty()) return {};
    return beam.front().firstMove;
}

}  // namespace fb::ai::beam
