#pragma once

#include <filesystem>
#include <vector>

#include "core/mino.hpp"

namespace fb::core {

// minos.json 形式のファイル(例: assets/minos_hard.json)を読み込み、Mino一覧を構築する。
std::vector<Mino> LoadMinos(const std::filesystem::path& jsonPath);

}  // namespace fb::core
