#include "cli/arg_parser.hpp"

#include <cctype>
#include <cmath>
#include <stdexcept>

namespace fb::cli {

void ArgParser::AddFlag(const std::string& name, FlagHandler handler) { flags_[name] = std::move(handler); }

void ArgParser::AddValue(const std::string& name, ValueHandler handler) { values_[name] = std::move(handler); }

void ArgParser::Parse(int argc, char** argv) const {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (auto flag = flags_.find(arg); flag != flags_.end()) {
            flag->second();
            continue;
        }

        if (auto option = values_.find(arg); option != values_.end()) {
            if (i + 1 >= argc) throw std::invalid_argument(arg + " には値が必要です");
            const std::string value = argv[++i];
            try {
                option->second(value);
            } catch (const std::invalid_argument& e) {
                throw std::invalid_argument(arg + " の値が不正です: " + e.what());
            }
            continue;
        }

        throw std::invalid_argument("未知のオプションです: " + arg);
    }
}

int ParseInt(const std::string& text) {
    std::size_t pos = 0;
    try {
        int value = std::stoi(text, &pos);
        if (pos == text.size()) return value;
    } catch (const std::exception&) {
    }
    throw std::invalid_argument("整数ではありません: " + text);
}

std::uint64_t ParseUint64(const std::string& text) {
    if (!text.empty() && std::isdigit(static_cast<unsigned char>(text[0]))) {
        std::size_t pos = 0;
        try {
            std::uint64_t value = std::stoull(text, &pos);
            if (pos == text.size()) return value;
        } catch (const std::exception&) {
        }
    }
    throw std::invalid_argument("0以上の整数ではありません: " + text);
}

float ParseFloat(const std::string& text) {
    std::size_t pos = 0;
    try {
        float value = std::stof(text, &pos);
        if (pos == text.size() && std::isfinite(value)) return value;
    } catch (const std::exception&) {
    }
    throw std::invalid_argument("数値ではありません: " + text);
}

}  // namespace fb::cli
