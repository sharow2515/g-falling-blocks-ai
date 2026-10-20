#include "cli/common_options.hpp"

namespace fb::cli {

void AddMinosOptions(ArgParser& parser, std::string& minosJsonPath) {
    parser.AddFlag("--easy", [&minosJsonPath] { minosJsonPath = kEasyMinosJsonPath; });
    parser.AddFlag("--hard", [&minosJsonPath] { minosJsonPath = kHardMinosJsonPath; });
}

void AddAiOptions(ArgParser& parser, ai::AiOptions& options, std::optional<std::string>* firstGiven) {
    auto noteGiven = [firstGiven](const std::string& name) {
        if (firstGiven != nullptr && !firstGiven->has_value()) *firstGiven = name;
    };

    parser.AddValue("--ai", [&options, noteGiven](const std::string& value) {
        noteGiven("--ai");
        options.name = value;
    });
    parser.AddValue("--beam-width", [&options, noteGiven](const std::string& value) {
        noteGiven("--beam-width");
        options.beamWidth = ParseInt(value);
    });
    parser.AddValue("--beam-depth", [&options, noteGiven](const std::string& value) {
        noteGiven("--beam-depth");
        options.beamDepth = ParseInt(value);
    });
    parser.AddValue("--weights", [&options, noteGiven](const std::string& value) {
        noteGiven("--weights");
        options.weights = value;
    });
}

}  // namespace fb::cli
