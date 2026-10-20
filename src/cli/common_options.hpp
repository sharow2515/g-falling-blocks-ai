#pragma once

#include <optional>
#include <string>

#include "ai/factory.hpp"
#include "cli/arg_parser.hpp"

namespace fb::cli {

inline constexpr const char* kHardMinosJsonPath = "assets/minos_hard.json";
inline constexpr const char* kEasyMinosJsonPath = "assets/minos_easy.json";

void AddMinosOptions(ArgParser& parser, std::string& minosJsonPath);
void AddAiOptions(ArgParser& parser, ai::AiOptions& options, std::optional<std::string>* firstGiven = nullptr);

}  // namespace fb::cli
