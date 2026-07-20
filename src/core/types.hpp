#pragma once

#include <cstdint>
#include <optional>

namespace fb::core {

struct Vec2i {
    int x = 0;
    int y = 0;

    friend Vec2i operator+(const Vec2i& a, const Vec2i& b) { return {a.x + b.x, a.y + b.y}; }
    friend Vec2i operator-(const Vec2i& a, const Vec2i& b) { return {a.x - b.x, a.y - b.y}; }
    friend bool operator==(const Vec2i& a, const Vec2i& b) { return a.x == b.x && a.y == b.y; }
    friend bool operator!=(const Vec2i& a, const Vec2i& b) { return !(a == b); }

    static Vec2i Up() { return {0, 1}; }
    static Vec2i Down() { return {0, -1}; }
    static Vec2i Left() { return {-1, 0}; }
    static Vec2i Right() { return {1, 0}; }
};

struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

}  // namespace fb::core
