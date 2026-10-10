#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "ai/factory.hpp"
#include "ai/player.hpp"
#include "app/ai_visualize_application.hpp"
#include "app/human_play_application.hpp"

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


fb::ai::AiOptions ResolveAiOptions(int argc, char** argv) {
    fb::ai::AiOptions options;
    if (auto name = ResolveStringOption(argc, argv, "--ai")) options.name = *name;
    options.beamWidth = ResolveIntOption(argc, argv, "--beam-width", options.beamWidth);
    options.beamDepth = ResolveIntOption(argc, argv, "--beam-depth", options.beamDepth);
    options.weights = ResolveStringOption(argc, argv, "--weights");
    return options;
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

// --visualize のときだけ意味を持つオプション。--visualize なしで渡されたら見つけて返す。
std::optional<std::string> FindAiOnlyOption(int argc, char** argv) {
    const char* aiOnlyOptions[] = {"--ai", "--beam-width", "--beam-depth", "--weights", "--ai-delay"};
    for (int i = 1; i < argc; ++i) {
        for (const char* option : aiOnlyOptions) {
            if (std::string(argv[i]) == option) return std::string(option);
        }
    }
    return std::nullopt;
}

}  // namespace

int main(int argc, char** argv) {
    std::string minosJsonPath = ResolveMinosJsonPath(argc, argv);
    std::optional<std::uint64_t> seed = ResolveSeed(argc, argv);

    if (!HasVisualizeFlag(argc, argv)) {
        if (auto option = FindAiOnlyOption(argc, argv)) {
            std::cerr << *option << " は --visualize と一緒に指定してください\n";
            return 1;
        }
        fb::app::HumanPlayApplication app(minosJsonPath, seed);
        return app.Run();
    }

    std::unique_ptr<fb::ai::AiPlayer> aiPlayer;
    try {
        aiPlayer = fb::ai::MakeAiPlayer(ResolveAiOptions(argc, argv));
    } catch (const std::invalid_argument& e) {
        std::cerr << "AI設定が不正です: " << e.what() << "\n";
        return 1;
    }

    fb::app::AiVisualizeApplication app(minosJsonPath, std::move(aiPlayer), ResolveAiDelay(argc, argv), seed);
    return app.Run();
}
