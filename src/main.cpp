#include <string>

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

}  // namespace

int main(int argc, char** argv) {
    fb::app::Application app(ResolveMinosJsonPath(argc, argv));
    return app.Run();
}
