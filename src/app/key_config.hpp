#pragma once

#include <SFML/Window/Keyboard.hpp>

namespace fb::app {

enum class KeyAction {
    MoveLeft,
    MoveRight,
    SoftDrop,
    HardDrop,
    RotateCW,
    RotateCCW,
    Hold,
    Pause,
    Restart,
    ToggleAi,
    Count,
};

inline sf::Keyboard::Key DefaultKeyFor(KeyAction action) {
    switch (action) {
        case KeyAction::MoveLeft: return sf::Keyboard::Key::Left;
        case KeyAction::MoveRight: return sf::Keyboard::Key::Right;
        case KeyAction::SoftDrop: return sf::Keyboard::Key::Down;
        case KeyAction::HardDrop: return sf::Keyboard::Key::Space;
        case KeyAction::RotateCW: return sf::Keyboard::Key::X;
        case KeyAction::RotateCCW: return sf::Keyboard::Key::Z;
        case KeyAction::Hold: return sf::Keyboard::Key::C;
        case KeyAction::Pause: return sf::Keyboard::Key::Escape;
        case KeyAction::Restart: return sf::Keyboard::Key::R;
        case KeyAction::ToggleAi: return sf::Keyboard::Key::A;
        default: return sf::Keyboard::Key::Unknown;
    }
}

}  // namespace fb::app
