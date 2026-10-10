#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>

namespace fb::cli {

class ArgParser {
public:
    using FlagHandler = std::function<void()>;
    using ValueHandler = std::function<void(const std::string& value)>;

    void AddFlag(const std::string& name, FlagHandler handler);
    void AddValue(const std::string& name, ValueHandler handler);
    void Parse(int argc, char** argv) const;

private:
    std::map<std::string, FlagHandler> flags_;
    std::map<std::string, ValueHandler> values_;
};

int ParseInt(const std::string& text);
std::uint64_t ParseUint64(const std::string& text);
float ParseFloat(const std::string& text);

}  // namespace fb::cli
