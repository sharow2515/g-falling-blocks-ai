#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "ai/factory.hpp"
#include "ai/player.hpp"
#include "benchmark/runner.hpp"
#include "cli/arg_parser.hpp"
#include "cli/common_options.hpp"
#include "core/mino_loader.hpp"

namespace {

struct Options {
    int games = 100;
    std::uint64_t seedStart = 1;
    int maxLines = 500;
    std::string minosJsonPath = fb::cli::kHardMinosJsonPath;
    fb::ai::AiOptions ai;
    std::string outputCsvPath;  // 空文字ならCSV出力しない
};

Options ParseOptions(int argc, char** argv) {
    Options opts;

    fb::cli::ArgParser parser;
    fb::cli::AddMinosOptions(parser, opts.minosJsonPath);
    fb::cli::AddAiOptions(parser, opts.ai);
    parser.AddValue("--games", [&opts](const std::string& value) { opts.games = fb::cli::ParseInt(value); });
    parser.AddValue("--seed-start",
                    [&opts](const std::string& value) { opts.seedStart = fb::cli::ParseUint64(value); });
    parser.AddValue("--max-lines", [&opts](const std::string& value) { opts.maxLines = fb::cli::ParseInt(value); });
    parser.AddValue("--output", [&opts](const std::string& value) { opts.outputCsvPath = value; });

    parser.Parse(argc, argv);
    return opts;
}

}  // namespace

int main(int argc, char** argv) {
    Options opts;
    try {
        opts = ParseOptions(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "オプションが不正です: " << e.what() << "\n";
        return 1;
    }

    std::vector<fb::core::Mino> minos;
    try {
        minos = fb::core::LoadMinos(opts.minosJsonPath);
    } catch (const std::exception& e) {
        std::cerr << "ミノ定義の読み込みに失敗しました: " << e.what() << "\n";
        return 1;
    }

    std::unique_ptr<fb::ai::AiPlayer> player;
    try {
        player = fb::ai::MakeAiPlayer(opts.ai);
    } catch (const std::invalid_argument& e) {
        std::cerr << "AI設定が不正です: " << e.what() << "\n";
        return 1;
    }

    std::ofstream csv;
    if (!opts.outputCsvPath.empty()) {
        csv.open(opts.outputCsvPath, std::ios::out | std::ios::trunc);
        if (!csv) {
            std::cerr << "出力ファイルを開けませんでした: " << opts.outputCsvPath << "\n";
            return 1;
        }
        csv << "seed,score,lines,hit_line_limit,avg_decision_ms\n";
    }

    double scoreSum = 0.0;
    double linesSum = 0.0;
    double decisionTimeMsSum = 0.0;
    int piecesSum = 0;
    int bestScore = 0;
    int worstScore = -1;
    int lineLimitHits = 0;

    for (int i = 0; i < opts.games; ++i) {
        std::uint64_t seed = opts.seedStart + static_cast<std::uint64_t>(i);
        fb::benchmark::GameResult result = fb::benchmark::RunOneGame(*player, minos, seed, opts.maxLines);

        scoreSum += result.score;
        linesSum += result.totalLines;
        decisionTimeMsSum += result.decisionTimeMs;
        piecesSum += result.totalPieces;
        if (result.score > bestScore) bestScore = result.score;
        if (worstScore < 0 || result.score < worstScore) worstScore = result.score;
        if (result.hitLineLimit) ++lineLimitHits;

        if (csv.is_open()) {
            double gameAvgDecisionMs = result.totalPieces > 0 ? result.decisionTimeMs / result.totalPieces : 0.0;
            csv << seed << "," << result.score << "," << result.totalLines << "," << (result.hitLineLimit ? 1 : 0)
                << "," << gameAvgDecisionMs << "\n";
            csv.flush();
        }
    }

    double avgScore = opts.games > 0 ? scoreSum / opts.games : 0.0;
    double avgLines = opts.games > 0 ? linesSum / opts.games : 0.0;
    double avgDecisionMs = piecesSum > 0 ? decisionTimeMsSum / piecesSum : 0.0;

    std::cout << "ai: " << player->Name() << "\n";
    std::cout << "games: " << opts.games << "\n";
    std::cout << "average score: " << avgScore << "\n";
    std::cout << "average lines: " << avgLines << "\n";
    std::cout << "average decision time (ms/piece): " << avgDecisionMs << "\n";
    std::cout << "best score: " << bestScore << "\n";
    std::cout << "worst score: " << worstScore << "\n";
    if (lineLimitHits > 0) {
        std::cout << lineLimitHits << " game(s) hit the --max-lines limit (" << opts.maxLines << ")\n";
    }

    return 0;
}
