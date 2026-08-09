#include "benchmark/runner.hpp"

#include <chrono>

#include "ai/common/move_execution.hpp"
#include "core/game.hpp"

namespace fb::benchmark {

namespace {
constexpr int kIterationBackstop = 10'000'000;
}  // namespace

GameResult RunOneGame(ai::AiPlayer& player, const std::vector<core::Mino>& minos, std::uint64_t seed,
                      int maxLines) {
    core::Game game(minos);  // Gameがallminosを所有するため、呼び出し側の一覧をコピーして渡す。
    game.StartGame(seed);

    int piecesPlaced = 0;
    double decisionTimeMs = 0.0;

    while (!game.IsGameOver() && game.GetTotalLines() < maxLines && piecesPlaced < kIterationBackstop) {
        auto start = std::chrono::steady_clock::now();
        std::vector<ai::PlayerMove> plan = player.Plan(game);
        auto end = std::chrono::steady_clock::now();
        decisionTimeMs += std::chrono::duration<double, std::milli>(end - start).count();

        if (plan.empty()) break;
        for (ai::PlayerMove move : plan) {
            ai::common::ExecuteMove(game, move);
        }
        ++piecesPlaced;
    }

    GameResult result;
    result.score = game.GetScore();
    result.totalLines = game.GetTotalLines();
    result.hitLineLimit = !game.IsGameOver() && result.totalLines >= maxLines;
    result.totalPieces = piecesPlaced;
    result.decisionTimeMs = decisionTimeMs;
    return result;
}

}  // namespace fb::benchmark
