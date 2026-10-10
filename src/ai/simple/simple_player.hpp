#pragma once

#include <string>
#include <vector>

#include "ai/player.hpp"
#include "core/game.hpp"

namespace fb::ai {

// 評価関数ベースの1手読みAI。Holdは使わない。
// 具体的な評価関数(現在はPierre Dellacherie (2003)の6特徴量)はevaluator.hppに分離してあり、
// 将来別の評価関数に差し替えても本クラスの名前・インターフェースは変わらない想定。
// SFML等appへの依存はなく、core::Gameのコマンドだけを叩く。
class SimplePlayer : public AiPlayer {
public:
    std::vector<PlayerMove> Plan(core::Game& game) override;
    std::string Name() const override { return "Simple"; }

private:
    std::vector<PlayerMove> BuildPlan(core::Game& game);
};

}  // namespace fb::ai
