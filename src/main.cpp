#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "ai/beam/beam_search_player.hpp"
#include "ai/beam/evaluator.hpp"
#include "ai/beam_tuned/beam_search_player.hpp"
#include "ai/beam_tuned/evaluator.hpp"
#include "ai/player.hpp"
#include "ai/reachability/reachability_player.hpp"
#include "ai/simple/simple_player.hpp"
#include "app/ai_visualize_application.hpp"
#include "app/application.hpp"

namespace {

// --easy: 7種(minos_easy.json)。指定なし/--hard: 全20種(minos_hard.json、デフォルト)。
std::string ResolveMinosJsonPath(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--easy") return "assets/minos_easy.json";
        if (arg == "--hard") return "assets/minos_hard.json";
    }
    return "assets/minos_hard.json";
}

int ResolveIntOption(int argc, char** argv, const std::string& flag, int defaultValue) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == flag && i + 1 < argc) {
            return std::atoi(argv[++i]);
        }
    }
    return defaultValue;
}

std::optional<std::string> ResolveStringOption(int argc, char** argv, const std::string& flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == flag && i + 1 < argc) {
            return std::string(argv[++i]);
        }
    }
    return std::nullopt;
}

// --ai simple(デフォルト) / --ai reachability / --ai beam / --ai beam_tuned
//   (beam/beam_tunedは --beam-width N / --beam-depth N (0〜5) / --weights を追加指定可能。
//    --weightsの個数はai::beam::Weights/ai::beam_tuned::Weightsの次元数と同じ)。
std::unique_ptr<fb::ai::AiPlayer> ResolveAiPlayer(int argc, char** argv) {
    std::string name = "simple";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ai" && i + 1 < argc) {
            name = argv[++i];
            break;
        }
    }

    if (name == "reachability") return std::make_unique<fb::ai::reachability::ReachabilityPlayer>();
    if (name == "beam") {
        int beamWidth = ResolveIntOption(argc, argv, "--beam-width", 20);
        int beamDepth = ResolveIntOption(argc, argv, "--beam-depth", 2);
        fb::ai::beam::Weights weights;
        if (auto arg = ResolveStringOption(argc, argv, "--weights")) {
            weights = fb::ai::beam::ParseWeights(*arg);
        }
        return std::make_unique<fb::ai::beam::BeamSearchPlayer>(beamWidth, beamDepth, weights);
    }
    if (name == "beam_tuned") {
        int beamWidth = ResolveIntOption(argc, argv, "--beam-width", 20);
        int beamDepth = ResolveIntOption(argc, argv, "--beam-depth", 2);
        fb::ai::beam_tuned::Weights weights;
        if (auto arg = ResolveStringOption(argc, argv, "--weights")) {
            weights = fb::ai::beam_tuned::ParseWeights(*arg);
        }
        return std::make_unique<fb::ai::beam_tuned::BeamSearchPlayer>(beamWidth, beamDepth, weights);
    }
    return std::make_unique<fb::ai::SimplePlayer>();
}

// --visualize: 人間操作を持たない、AI観戦専用モードで起動する。
bool HasVisualizeFlag(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--visualize") return true;
    }
    return false;
}

// --ai-delay N: visualizeモードで1操作ごとに置く間隔の秒数。
float ResolveAiDelay(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ai-delay" && i + 1 < argc) {
            return static_cast<float>(std::atof(argv[++i]));
        }
    }
    return 0.3f;
}

// --seed N: ミノの出現列を固定する(Restartでも同じシードを使い直す)。未指定ならランダム。
std::optional<std::uint64_t> ResolveSeed(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc) {
            return static_cast<std::uint64_t>(std::atoll(argv[++i]));
        }
    }
    return std::nullopt;
}

}  // namespace

int main(int argc, char** argv) {
    std::string minosJsonPath = ResolveMinosJsonPath(argc, argv);
    std::optional<std::uint64_t> seed = ResolveSeed(argc, argv);

    std::unique_ptr<fb::ai::AiPlayer> aiPlayer;
    try {
        aiPlayer = ResolveAiPlayer(argc, argv);
    } catch (const std::invalid_argument& e) {
        std::cerr << "AI設定が不正です: " << e.what() << "\n";
        return 1;
    }

    if (HasVisualizeFlag(argc, argv)) {
        fb::app::AiVisualizeApplication app(minosJsonPath, std::move(aiPlayer), ResolveAiDelay(argc, argv), seed);
        return app.Run();
    }

    fb::app::Application app(minosJsonPath, std::move(aiPlayer), seed);
    return app.Run();
}
