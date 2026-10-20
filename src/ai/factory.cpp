#include "ai/factory.hpp"

#include <stdexcept>
#include <string_view>

#include "ai/beam/beam_search_player.hpp"
#include "ai/beam/evaluator.hpp"
#include "ai/beam_tuned/beam_search_player.hpp"
#include "ai/beam_tuned/evaluator.hpp"
#include "ai/beam_unique/beam_search_player.hpp"
#include "ai/beam_unique/evaluator.hpp"
#include "ai/reachability/reachability_player.hpp"
#include "ai/simple/simple_player.hpp"

namespace fb::ai {

namespace {

template <typename Player, typename Weights>
std::unique_ptr<AiPlayer> MakeBeamPlayer(const AiOptions& options, Weights (*parseWeights)(const std::string&)) {
    Weights weights;
    if (options.weights) weights = parseWeights(*options.weights);
    return std::make_unique<Player>(options.beamWidth, options.beamDepth, weights);
}

struct AiEntry {
    std::string_view name;
    std::unique_ptr<AiPlayer> (*make)(const AiOptions&);
};

const AiEntry kAiEntries[] = {
    {"simple", [](const AiOptions&) -> std::unique_ptr<AiPlayer> { return std::make_unique<SimplePlayer>(); }},
    {"reachability",
     [](const AiOptions&) -> std::unique_ptr<AiPlayer> { return std::make_unique<reachability::ReachabilityPlayer>(); }},
    {"beam", [](const AiOptions& o) { return MakeBeamPlayer<beam::BeamSearchPlayer>(o, beam::ParseWeights); }},
    {"beam_tuned",
     [](const AiOptions& o) { return MakeBeamPlayer<beam_tuned::BeamSearchPlayer>(o, beam_tuned::ParseWeights); }},
    {"beam_unique",
     [](const AiOptions& o) { return MakeBeamPlayer<beam_unique::BeamSearchPlayer>(o, beam_unique::ParseWeights); }},
};

}  // namespace

std::unique_ptr<AiPlayer> MakeAiPlayer(const AiOptions& options) {
    for (const AiEntry& entry : kAiEntries) {
        if (entry.name == options.name) return entry.make(options);
    }

    std::string message = "未知のAI名です: " + options.name + " (指定できるAI名: ";
    for (const AiEntry& entry : kAiEntries) {
        if (&entry != &kAiEntries[0]) message += ", ";
        message += entry.name;
    }
    message += ")";
    throw std::invalid_argument(message);
}

}  // namespace fb::ai
