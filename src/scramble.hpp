#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>

inline std::string random_scramble(int count, std::uint32_t seed) {
    if (count < 0)
        throw std::invalid_argument("scramble count must be non-negative");

    constexpr std::array<const char*, 18> moves{
        "U", "U'", "U2", "R", "R'", "R2", "F", "F'", "F2",
        "D", "D'", "D2", "L", "L'", "L2", "B", "B'", "B2"
    };
    std::mt19937 rng(seed);
    std::string result;
    for (int i = 0; i < count; ++i) {
        if (!result.empty()) result += ' ';
        result += moves[rng() % moves.size()];
    }
    return result;
}
