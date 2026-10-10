#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "ai/factory.hpp"
#include "app/ai_visualize_application.hpp"
#include "app/human_play_application.hpp"
#include "cli/arg_parser.hpp"
#include "cli/common_options.hpp"

namespace {

struct Options {
    std::string minosJsonPath = fb::cli::kHardMinosJsonPath;
    std::optional<std::uint64_t> seed;
    bool visualize = false; // AIの操作を表示するモードにするか
    float aiDelay = 0.3f;
    fb::ai::AiOptions ai;
    std::optional<std::string> firstAiOption;
};

Options ParseOptions(int argc, char** argv) {
    Options opts;

    fb::cli::ArgParser parser;
    fb::cli::AddMinosOptions(parser, opts.minosJsonPath);
    fb::cli::AddAiOptions(parser, opts.ai, &opts.firstAiOption);
    parser.AddFlag("--visualize", [&opts] { opts.visualize = true; });
    parser.AddValue("--seed", [&opts](const std::string& value) { opts.seed = fb::cli::ParseUint64(value); });
    parser.AddValue("--ai-delay", [&opts](const std::string& value) {
        if (!opts.firstAiOption) opts.firstAiOption = "--ai-delay";
        opts.aiDelay = fb::cli::ParseFloat(value);
    });

    parser.Parse(argc, argv);
    return opts;
}

}  // namespace

int main(int argc, char** argv) {
    Options opts;
    try {
        opts = ParseOptions(argc, argv);
    } catch (const std::invalid_argument& e) {
        std::cerr << "引数が不正です: " << e.what() << "\n";
        return 1;
    }

    if (!opts.visualize) {
        if (opts.firstAiOption) {
            std::cerr << *opts.firstAiOption << " は --visualize と一緒に指定してください\n";
            return 1;
        }
        fb::app::HumanPlayApplication app(opts.minosJsonPath, opts.seed);
        return app.Run();
    }

    std::unique_ptr<fb::ai::AiPlayer> aiPlayer;
    try {
        aiPlayer = fb::ai::MakeAiPlayer(opts.ai);
    } catch (const std::invalid_argument& e) {
        std::cerr << "AI設定が不正です: " << e.what() << "\n";
        return 1;
    }

    fb::app::AiVisualizeApplication app(opts.minosJsonPath, std::move(aiPlayer), opts.aiDelay, opts.seed);
    return app.Run();
}
