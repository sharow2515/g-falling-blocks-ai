#include "core/mino_loader.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace fb::core {

namespace {

Color ParseHexColor(const std::string& hex) {
    // "#RRGGBB" 形式のみ想定 (minos.json の全エントリがこの形式)。
    if (hex.size() != 7 || hex[0] != '#') {
        throw std::runtime_error("MinoLoader: 不正な色フォーマット: " + hex);
    }
    auto byteAt = [&](size_t pos) {
        return static_cast<std::uint8_t>(std::stoi(hex.substr(pos, 2), nullptr, 16));
    };
    return Color{byteAt(1), byteAt(3), byteAt(5)};
}

}  // namespace

std::vector<Mino> LoadMinos(const std::filesystem::path& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("MinoLoader: ファイルを開けません: " + jsonPath.string());
    }

    nlohmann::json root;
    file >> root;

    std::vector<Mino> minos;
    minos.reserve(root.size());

    int nextId = 0;
    for (const auto& entry : root) {
        Mino mino;
        mino.id = nextId++;
        mino.name = entry.at("name").get<std::string>();
        mino.isOriginal = entry.at("isOriginal").get<bool>();
        mino.gridSize = entry.at("gridSize").get<int>();
        mino.color = ParseHexColor(entry.at("color").get<std::string>());

        for (const auto& cell : entry.at("cells")) {
            mino.baseCells.push_back(Vec2i{cell.at(0).get<int>(), cell.at(1).get<int>()});
        }

        minos.push_back(std::move(mino));
    }

    if (minos.empty()) {
        throw std::runtime_error("MinoLoader: ミノが1つも読み込めませんでした: " + jsonPath.string());
    }

    return minos;
}

}  // namespace fb::core
