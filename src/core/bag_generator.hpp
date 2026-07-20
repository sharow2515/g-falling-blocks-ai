#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "core/mino.hpp"

namespace fb::core {

class BagGenerator {
public:
    BagGenerator(std::vector<Mino> allMinos, std::uint64_t seed);

    void Reseed(std::uint64_t seed);

    std::vector<const Mino*> GenerateBag();

    [[nodiscard]] const std::vector<Mino>& AllMinos() const { return allMinos_; }

private:
    std::vector<Mino> allMinos_;
    std::mt19937 rng_;
};

}  // namespace fb::core
