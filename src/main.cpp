#include <cstdlib>
#include <memory>
#include <string>

#include "ai/heuristic/heuristic_player.hpp"
#include "ai/player.hpp"
#include "ai/reachability/reachability_player.hpp"
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

// --ai heuristic(デフォルト) / --ai reachability。
std::unique_ptr<fb::ai::AiPlayer> ResolveAiPlayer(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ai" && i + 1 < argc) {
            std::string name = argv[++i];
            if (name == "reachability") return std::make_unique<fb::ai::reachability::ReachabilityPlayer>();
            return std::make_unique<fb::ai::HeuristicPlayer>();
        }
    }
    return std::make_unique<fb::ai::HeuristicPlayer>();
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

}  // namespace

int main(int argc, char** argv) {
    std::string minosJsonPath = ResolveMinosJsonPath(argc, argv);

    if (HasVisualizeFlag(argc, argv)) {
        fb::app::AiVisualizeApplication app(minosJsonPath, ResolveAiPlayer(argc, argv),
                                             ResolveAiDelay(argc, argv));
        return app.Run();
    }

    fb::app::Application app(minosJsonPath, ResolveAiPlayer(argc, argv));
    return app.Run();
}
