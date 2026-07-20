#include "core/bag_generator.hpp"

namespace fb::core {

BagGenerator::BagGenerator(std::vector<Mino> allMinos, std::uint64_t seed)
    : allMinos_(std::move(allMinos)), rng_(static_cast<std::uint32_t>(seed)) {}

void BagGenerator::Reseed(std::uint64_t seed) {
    rng_.seed(static_cast<std::uint32_t>(seed));
}

std::vector<const Mino*> BagGenerator::GenerateBag() {
    std::vector<const Mino*> bag;
    bag.reserve(allMinos_.size());
    for (const auto& mino : allMinos_) {
        bag.push_back(&mino);
    }

    // Fisher-Yates
    for (int i = static_cast<int>(bag.size()) - 1; i > 0; --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng_);
        std::swap(bag[i], bag[j]);
    }

    return bag;
}

}  // namespace fb::core
