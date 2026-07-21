#include "benchmark/runner.hpp"

#include "core/game.hpp"

namespace fb::benchmark {

namespace {
// maxLinesに到達しない場合(AIが手詰まりで一切進行しない等)に備えたもの。
// 通常はmaxLines到達かゲームオーバーで先に止まる。
constexpr int kIterationBackstop = 10'000'000;
}  // namespace

GameResult RunOneGame(ai::AiPlayer& player, const std::vector<core::Mino>& minos, std::uint64_t seed,
                      int maxLines) {
    core::Game game(minos);  // Gameがallminosを所有するため、呼び出し側の一覧をコピーして渡す。
    game.StartGame(seed);

    int iterations = 0;
    while (!game.IsGameOver() && game.GetTotalLines() < maxLines && iterations < kIterationBackstop) {
        player.Update(game);
        ++iterations;
    }

    GameResult result;
    result.score = game.GetScore();
    result.totalLines = game.GetTotalLines();
    result.hitLineLimit = !game.IsGameOver() && result.totalLines >= maxLines;
    return result;
}

}  // namespace fb::benchmark
