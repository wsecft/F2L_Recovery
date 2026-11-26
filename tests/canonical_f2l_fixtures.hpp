#pragma once

#include <array>
#include <string_view>

// Canonical fixture data is derived from the MIT-licensed CFOP dataset:
// https://github.com/andyjudson/cfop

struct CanonicalF2LFixture {
    std::string_view id;
    std::string_view group;
    std::string_view setup;
    std::string_view solution;
};

inline constexpr std::string_view canonical_f2l_source =
    "https://github.com/andyjudson/cfop/blob/main/cfop-app/public/data/cfop-f2l.json";

inline constexpr std::array<CanonicalF2LFixture, 41> canonical_f2l_fixtures = {{
    {"f2l-1-1", "easy inserts", "z2", "U R U' R'"},
    {"f2l-1-2", "easy inserts", "z2", "y U' L' U L"},
    {"f2l-1-3", "easy inserts", "z2", "R U R'"},
    {"f2l-1-4", "easy inserts", "z2", "F' U' F"},
    {"f2l-2-1", "disconnected pairs", "z2", "U' R U R' U2 R U' R'"},
    {"f2l-2-2", "disconnected pairs", "z2", "y U L' U' L U2 L' U L"},
    {"f2l-2-3", "disconnected pairs", "z2", "U' R U2 R' U2 R U' R'"},
    {"f2l-2-4", "disconnected pairs", "z2", "y U L' U2 L U2 L' U L"},
    {"f2l-2-5", "disconnected pairs", "z2", "U' R U R' U R U R'"},
    {"f2l-2-6", "disconnected pairs", "z2", "U' R U' R' U F' U' F"},
    {"f2l-2-7", "disconnected pairs", "z2", "U R U2 R' U R U' R'"},
    {"f2l-2-8", "disconnected pairs", "z2", "y U' L' U2 L U' L' U L"},
    {"f2l-2-9", "disconnected pairs", "z2", "U2 R U R' U R U' R'"},
    {"f2l-2-10", "disconnected pairs", "z2", "y U2 L' U' L U' L' U L"},
    {"f2l-3-1", "corner in slot", "z2", "U' R' F R F' R U R'"},
    {"f2l-3-2", "corner in slot", "z2", "U R U' R' U' F' U F"},
    {"f2l-3-3", "corner in slot", "z2", "R U' R' U R U' R'"},
    {"f2l-3-4", "corner in slot", "z2", "y L' U L U' L' U L"},
    {"f2l-3-5", "corner in slot", "z2", "R U R' U' R U R'"},
    {"f2l-3-6", "corner in slot", "z2", "y L' U' L U L' U' L"},
    {"f2l-4-1", "edge in slot", "z2", "U R U' R' U R U' R' U R U' R'"},
    {"f2l-4-2", "edge in slot", "z2", "U' R' F R F' R U' R'"},
    {"f2l-4-3", "edge in slot", "z2", "U' R U' R' U2 R U' R'"},
    {"f2l-4-4", "edge in slot", "z2", "U' R U R' U F' U' F"},
    {"f2l-4-5", "edge in slot", "z2", "U R U R' U2 R U R'"},
    {"f2l-4-6", "edge in slot", "z2", "U F' U' F U' R U R'"},
    {"f2l-5-1", "connected pairs", "z2", "R' U2 R2 U R2 U R"},
    {"f2l-5-2", "connected pairs", "z2", "U' R U2 R' U F' U' F"},
    {"f2l-5-3", "connected pairs", "z2", "U' R U' R' U R U R'"},
    {"f2l-5-4", "connected pairs", "z2", "y U L' U L U' L' U' L"},
    {"f2l-5-5", "connected pairs", "z2", "R U R' U2 R U' R' U R U' R'"},
    {"f2l-5-6", "connected pairs", "z2", "R U' R' U2 F' U' F"},
    {"f2l-5-7", "connected pairs", "z2", "R U2 R' U' R U R'"},
    {"f2l-5-8", "connected pairs", "z2", "y L' U2 L U L' U' L"},
    {"f2l-5-9", "connected pairs", "z2", "U R U' R' U' R U' R' U R U' R'"},
    {"f2l-5-10", "connected pairs", "z2", "F U R U' R' F' R U' R'"},
    {"f2l-6-1", "pair in slot", "z2 y'", "R U' R' U y' R' U2 R U2 R' U R"},
    {"f2l-6-2", "pair in slot", "z2", "R U' R' U' R U R' U2 R U' R'"},
    {"f2l-6-3", "pair in slot", "z2", "R U' R' U' R U' R' U F' U' F"},
    {"f2l-6-4", "pair in slot", "z2", "R U' R' U R U2 R' U R U' R'"},
    {"f2l-6-5", "pair in slot", "z2 y", "R U' R' y U2 L' U' L U' L' U L"},
}};
