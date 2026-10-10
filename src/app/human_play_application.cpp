#include "app/human_play_application.hpp"

namespace fb::app {

HumanPlayApplication::HumanPlayApplication(const std::string& minosJsonPath, std::optional<std::uint64_t> seed)
    : ApplicationBase(minosJsonPath, seed, "FallingBlocks") {}

void HumanPlayApplication::Update(float deltaSeconds) {
    input_.Update(game_, deltaSeconds);
    TryRestart();
    game_.Tick(deltaSeconds);
}

}  // namespace fb::app
