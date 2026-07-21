#include <memory>
#include <string>

#include "ai/heuristic/heuristic_player.hpp"
#include "ai/player.hpp"
#include "ai/reachability/reachability_player.hpp"
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

}  // namespace

int main(int argc, char** argv) {
    fb::app::Application app(ResolveMinosJsonPath(argc, argv), ResolveAiPlayer(argc, argv));
    return app.Run();
}
