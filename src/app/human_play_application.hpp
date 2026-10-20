#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "app/application_base.hpp"

namespace fb::app {

class HumanPlayApplication : public ApplicationBase {
public:
    HumanPlayApplication(const std::string& minosJsonPath, std::optional<std::uint64_t> seed = std::nullopt);

private:
    void Update(float deltaSeconds) override;
};

}  // namespace fb::app
