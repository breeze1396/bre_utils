#pragma once

namespace bre {

// ANSI 颜色枚举
enum class Color {
    BLACK,
    BLUE,
    CYAN,
    GREEN,
    PURPLE,
    RED,
    WHITE,
    YELLOW,

    RESET,
};

inline const char* ColorToAnsi(Color color) {
    switch (color) {
        case Color::BLACK:
            return "\033[1;30m";
        case Color::RED:
            return "\033[1;31m";
        case Color::GREEN:
            return "\033[1;32m";
        case Color::YELLOW:
            return "\033[1;33m";
        case Color::BLUE:
            return "\033[1;34m";
        case Color::PURPLE:
            return "\033[1;35m";
        case Color::CYAN:
            return "\033[1;36m";
        case Color::WHITE:
            return "\033[1;37m";
        case Color::RESET:
        default:
            return "\033[0m";
    }
}


}  // namespace bre