#pragma once

#include <array>
#include <string_view>

struct RealSolvePhase {
    std::string_view label;
    std::string_view moves;
};

struct RealSolveFixture {
    std::string_view id;
    std::string_view name;
    std::string_view source_url;
    std::string_view setup;
    std::array<RealSolvePhase, 7> phases;
};

inline constexpr std::array<RealSolveFixture, 3> real_solve_fixtures = {{
    {
        "12872",
        "Seung Hyuk Nahm - 4.69 3x3",
        "https://reco.nz/solve/12872",
        "L2 U' F D' F' B2 U2 L B F2 L2 U F2 R2 D' R2 D2 L2 F",
        {{
            {"inspection", "x' z"},
            {"cross", "U D2 r' R D'"},
            {"1st pair", "R' U' R2"},
            {"2nd pair", "U R' d R' U' R"},
            {"3rd pair", "U' L' U L"},
            {"4th pair/ZBLS", "F R' F' R2 U R' U R' F R F'"},
            {"ZBLL", "U2' U R U2 R' U' R2 D R' U' R D' R2' U2'"}
        }}
    },
    {
        "13251",
        "Teodor Zajder - 3.84 3x3",
        "https://reco.nz/solve/13251",
        "U L2 D' F2 D2 U' B2 F2 U' R2 B' U' F R U' L2 R' U' B' L",
        {{
            {"inspection", "y'"},
            {"pseudo xxcross", "R' F D' U' R F2 R"},
            {"3rd pair/finish cross", "L' U2 y' R U R' U' R U R' U R' F2 R"},
            {"4th pair", "U2' L' U2 L U L' U' L"},
            {"ZBLL", "U' U F R U R' U' R U R' U' F' R U R' U' R' F R F' U2"},
            {"", ""},
            {"", ""}
        }}
    },
    {
        "12267",
        "Tymon Kolasiński - 6.81 3x3",
        "https://reco.nz/solve/12267",
        "R F2 L2 F L B2 R B D R' L F2 L B2 L' D2 L2 D'",
        {{
            {"inspection", "x2 y"},
            {"cross", "R' B' R2 F D' L"},
            {"1st pair", "D' D L' U' L D' U R U' R' D"},
            {"2nd pair", "L' U' L U L' U' L"},
            {"3rd pair", "U R U' R' d L' U L"},
            {"4th pair/ZBLS", "y U U2 F' U' L' U2 L U2' L' U L F"},
            {"ZBLL", "U' r U2' r' U2 L' U2 r U2' r' U2 L U2"}
        }}
    }
}};
