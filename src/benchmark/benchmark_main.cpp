#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "ai/beam/beam_search_player.hpp"
#include "ai/beam/evaluator.hpp"
#include "ai/beam_tuned/beam_search_player.hpp"
#include "ai/beam_tuned/evaluator.hpp"
#include "ai/player.hpp"
#include "ai/reachability/reachability_player.hpp"
#include "ai/simple/simple_player.hpp"
#include "benchmark/runner.hpp"
#include "core/mino_loader.hpp"

namespace {

struct Options {
    int games = 100;
    std::uint64_t seedStart = 1;
    int maxLines = 500;
    std::string minosJsonPath = "assets/minos_hard.json";
    std::string aiName = "simple";  // "simple" | "reachability" | "beam" | "beam_tuned"
    int beamWidth = 20;
    int beamDepth = 2;
    std::string outputCsvPath;  // 空文字ならCSV出力しない
    std::optional<std::string> weightsArg;  // --ai beam/beam_tuned のときのみ使用。未指定ならデフォルト重み
};

Options ParseOptions(int argc, char** argv) {
    Options opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--games" && i + 1 < argc) {
            opts.games = std::atoi(argv[++i]);
        } else if (arg == "--seed-start" && i + 1 < argc) {
            opts.seedStart = static_cast<std::uint64_t>(std::atoll(argv[++i]));
        } else if (arg == "--max-lines" && i + 1 < argc) {
            opts.maxLines = std::atoi(argv[++i]);
        } else if (arg == "--ai" && i + 1 < argc) {
            opts.aiName = argv[++i];
        } else if (arg == "--beam-width" && i + 1 < argc) {
            opts.beamWidth = std::atoi(argv[++i]);
        } else if (arg == "--beam-depth" && i + 1 < argc) {
            opts.beamDepth = std::atoi(argv[++i]);
        } else if (arg == "--easy") {
            opts.minosJsonPath = "assets/minos_easy.json";
        } else if (arg == "--hard") {
            opts.minosJsonPath = "assets/minos_hard.json";
        } else if (arg == "--output" && i + 1 < argc) {
            opts.outputCsvPath = argv[++i];
        } else if (arg == "--weights" && i + 1 < argc) {
            opts.weightsArg = argv[++i];
        }
    }
    return opts;
}

std::unique_ptr<fb::ai::AiPlayer> MakePlayer(const Options& opts) {
    if (opts.aiName == "reachability") return std::make_unique<fb::ai::reachability::ReachabilityPlayer>();
    if (opts.aiName == "beam") {
        fb::ai::beam::Weights weights;
        if (opts.weightsArg) weights = fb::ai::beam::ParseWeights(*opts.weightsArg);
        return std::make_unique<fb::ai::beam::BeamSearchPlayer>(opts.beamWidth, opts.beamDepth, weights);
    }
    if (opts.aiName == "beam_tuned") {
        fb::ai::beam_tuned::Weights weights;
        if (opts.weightsArg) weights = fb::ai::beam_tuned::ParseWeights(*opts.weightsArg);
        return std::make_unique<fb::ai::beam_tuned::BeamSearchPlayer>(opts.beamWidth, opts.beamDepth, weights);
    }
    return std::make_unique<fb::ai::SimplePlayer>();
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
        player = MakePlayer(opts);
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
