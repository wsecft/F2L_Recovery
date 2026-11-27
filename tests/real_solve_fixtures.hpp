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
    std::array<RealSolvePhase, 8> phases;
};

struct UnknownF2LFixture {
    std::string_view id;
    std::string_view name;
    std::string_view single_time;
    std::string_view recorded_at;
    std::string_view scramble;
    std::string_view inspection;
    std::string_view cross;
    std::string_view last_layer;
};

inline constexpr std::array<RealSolveFixture, 9> real_solve_fixtures = {{
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
    },
    {
        "13033",
        "Yi Shen - 4.66 3x3 (Xi'an Cherry Blossom 2026)",
        "https://reco.nz/solve/13033",
        "U B2 D L2 F2 D2 L2 B2 D' B' U' L2 F' L U2 R U F U",
        {{
            {"inspection", "x2 y"},
            {"cross", "U D' F R D r U' r'"},
            {"1st pair", "U R U R' U R U' R'"},
            {"2nd pair", "U R' U' R U R' U' R"},
            {"3rd pair", "L' U' L d R U' R'"},
            {"4th pair/ZBLS", "y' U' F R' F' R y F R U R' F'"},
            {"ZBLL", "R U2 R D r' U2 r D' R2' U2'"},
            {"", ""}
        }}
    },
    {
        "11961",
        "Teodor Zajder - 5.57 3x3 (WCA World Championship 2025)",
        "https://reco.nz/solve/11961",
        "R2 D' R2 U' F2 U' R2 U F2 D R' U2 L2 B R2 D R2 F2 U2 R",
        {{
            {"inspection", "y2"},
            {"xxcross", "D R2 U' r' U r U R' U' R2 U R2' F R U' R' F R D"},
            {"3rd pair", "U y' R' U2 R U' R' U R"},
            {"4th pair", "U' R U2' R' U R' F R F'"},
            {"OLL", "U2' F R' F' R U R U' R'"},
            {"PLL", "U' D' R2 U R' U R' U' R U' R2 D U' R' U R"},
            {"", ""},
            {"", ""}
        }}
    },
    {
        "12035",
        "Yiheng Wang - 3.62 3x3 (Taizhou Open 2025)",
        "https://reco.nz/solve/12035",
        "R2 U2 R2 F2 D' L2 D R2 D' L U2 L F2 R U2 F D' R B R'",
        {{
            {"inspection", "x'"},
            {"xxcross", "D R U R r' U2' L U y R' F2 U' R"},
            {"3rd pair", "U' f R f'"},
            {"4th pair", "U2 F U' F' U2 L' U' L"},
            {"OLL", "U2' r U R' U' r' F R F'"},
            {"PLL", "R U R' U R U R' F' R U R' U' R' F R2 U' R' U2 R U' R'"},
            {"", ""},
            {"", ""}
        }}
    },
    {
        "12488",
        "Xuanyi Geng - 5.09 3x3 (Beijing Winter 2026)",
        "https://reco.nz/solve/12488",
        "D F2 U' L2 D2 L2 D2 F2 R B2 R' B R' B2 F2 D' F L' R'",
        {{
            {"inspection", "x z"},
            {"cross", "U' R' D' r D' L"},
            {"1st pair", "d L U L'"},
            {"2nd pair", "U' L' U2 L U L' U' L"},
            {"3rd pair", "U' F' R' U' R F"},
            {"4th pair/ZBLS", "U F R' F' R2 U' R' U R U R'"},
            {"ZBLL", "U' U2' R2 D r' U2 r R' U' R D' R' U' R' U2'"},
            {"", ""}
        }}
    },
    {
        "14031",
        "Xuanyi Geng - 3.85 3x3 (Jiajiang Open 2026)",
        "https://reco.nz/solve/14031",
        "D' F U L R2 D2 R2 D2 R' D2 R' D2 U R2 U' R D U B'",
        {{
            {"inspection", "x z"},
            {"cross", "F' D r R' D2"},
            {"1st pair", "L U L'"},
            {"2nd pair", "D U R' U' R D'"},
            {"3rd pair", "y R U' R' U' L U L'"},
            {"4th pair/ZBLS", "R R' U R U R' U R U' R2' F R F'"},
            {"ZBLL", "U R' U2' R U R' U R2 U2' R' U' R U' R' U'"},
            {"", ""}
        }}
    },
    {
        "cubedb-5.58",
        "3x3 - 5.58 (CubeDB user-provided reconstruction)",
        "https://cubedb.net/?puzzle=3x3&scramble=D_R-_U-_L2_D-_F2_R2_B2_U_B2_L2_D-_F2_R-_D-_B_D-_U_F-_D2&time=5.58&alg=z2_%2F%2F_inspection%0AB_R-_F_R_L_D-_%2F%2F_cross_(6)%0AL_U-_L-_%2F%2F_1st_pair_(3)%0AU-_R-_U2_R_L-_U_L__%2F%2F_2nd_pair_(7)%0AU2_R_U-_R-_%2F%2F_3rd_pair_(4)%0Ay_U-_R_U_R-_%2F%2F_4th_pair_(5)%0AU_F_R-_F-_R_U_R_U-_R-_%2F%2F_OLL_37%0AU_R2_U-_R_U-_R_U_R-_U_R2_D-_U_R_U-_R-_D_U-_%2F%2F_PLL_-_Gc_",
        "D R' U' L2 D' F2 R2 B2 U B2 L2 D' F2 R' D' B D' U F' D2",
        {{
            {"inspection", "z2"},
            {"cross", "B R' F R L D'"},
            {"1st pair", "L U' L'"},
            {"2nd pair", "U' R' U2 R L' U L"},
            {"3rd pair", "U2 R U' R'"},
            {"4th pair", "y U' R U R'"},
            {"OLL", "U F R' F' R U R U' R'"},
            {"PLL", "U R2 U' R U' R U R' U R2 D' U R U' R' D U'"}
        }}
    }
}};

inline constexpr UnknownF2LFixture unknown_f2l_fixture = {
    "cstimer-2025-05-29",
    "PB!!!!!! (PLL skip)",
    "5.59",
    "2025-05-29 21:51:43",
    "L2 U2 L D' F R F' R2 D2 L' B2 R' D2 R2 U2 L B2 R D B",
    "x2",
    "U' L F' L2 R' B2",
    "F R' F' r U R U' r' U'"
};
