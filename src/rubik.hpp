#pragma once
#include <array>
#include <string_view>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <frozen/unordered_map.h>
#pragma message("Including rubik.hpp")


struct Cube {
	// If it is unclear, the permutation means the index of the piece in the solved cube, so each value in the arrays correspond to a physical slot but NOT to a specific piece.
    uint8_t corner_perm[8];   // permutation of 8 corners (values 0-7)
    uint8_t corner_orient[8]; // orientation of each corner (values 0-2)
    uint8_t edge_perm[12];    // permutation of 12 edges (values 0-11)
    uint8_t edge_orient[12];  // orientation of each edge (values 0-1)
    constexpr static Cube identity() {
        Cube solved_cube;
        for (int i = 0; i < 8; i++) {
            solved_cube.corner_perm[i] = i;
            solved_cube.corner_orient[i] = 0;
        }
        for (int i = 0; i < 12; i++) {
            solved_cube.edge_perm[i] = i;
            solved_cube.edge_orient[i] = 0;
        }
		return solved_cube;
    }
    enum sides {
        U, R, F, D, L, B
    };

        // Corner and edge definitions (facelets affected by each piece)
        inline static const int corner_facelets[8][3] = {
            {U, R, F}, {U, F, L}, {U, L, B}, {U, B, R},
            {D, F, R}, {D, L, F}, {D, B, L}, {D, R, B}
        };

        inline static const int edge_facelets[12][2] = {
            {U, F}, {U, R}, {U, B}, {U, L},
            {D, F}, {D, R}, {D, B}, {D, L},
            {F, R}, {B, R}, {B, L}, {F, L}
        };
    bool parity() const {
        // Calculate the parity of the cube based on corner and edge permutations
        int corner_parity = 0;
        for (int i = 0; i < 8; i++) {
            for (int j = i + 1; j < 8; j++) {
                if (corner_perm[i] > corner_perm[j]) {
                    corner_parity++;
                }
            }
        }
        int edge_parity = 0;
        for (int i = 0; i < 12; i++) {
            for (int j = i + 1; j < 12; j++) {
                if (edge_perm[i] > edge_perm[j]) {
                    edge_parity++;
                }
            }
        }
        return (corner_parity + edge_parity) % 2 != 0;
	}
};

struct Move {
    std::array<uint8_t, 8> corner_perm{};
    std::array<uint8_t, 8> corner_orient{};
    std::array<uint8_t, 12> edge_perm{};
    std::array<uint8_t, 12> edge_orient{};

    constexpr bool operator==(const Move&) const = default;
    constexpr static Move identity() {
        return Move{
            {0,1,2,3,4,5,6,7},       // corner_perm
            {0,0,0,0,0,0,0,0},       // corner_orient
            {0,1,2,3,4,5,6,7,8,9,10,11}, // edge_perm
            {0,0,0,0,0,0,0,0,0,0,0,0}    // edge_orient
        };
    }
};

std::ostream& operator<<(std::ostream&, Move m);
std::ostream& operator<<(std::ostream&, Cube m);
void apply_move(Cube& cube, const Move& m);
constexpr Move operator+(const Move& m1, const Move& m2);
constexpr Move operator*(const Move&, const int);
constexpr Move operator-(const Move& m);

struct Rubik {
    // --- Define your six base moves as constexpr ---
    static constexpr Move U = Move{
        {3,0,1,2,4,5,6,7},
        {0,0,0,0,0,0,0,0},
        {1,2,3,0,4,5,6,7,8,9,10,11},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };

    static constexpr Move R = Move{
        {4,1,2,0,7,5,6,3},
        {1,0,0,2,2,0,0,1},
        {0,8,2,3,4,9,6,7,5,1,10,11},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };

    static constexpr Move F = Move{
        {1,5,2,3,0,4,6,7},
        {2,1,0,0,1,2,0,0},
        {11,1,2,3,8,5,6,7,0,9,10,4},
        {1,0,0,0,1,0,0,0,1,0,0,1}
    };

    static constexpr Move D = Move{
        {0,1,2,3,5,6,7,4},
        {0,0,0,0,0,0,0,0},
        {0,1,2,3,7,4,5,6,8,9,10,11},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };

    static constexpr Move L = Move{
        {0,2,6,3,4,1,5,7},
        {0,2,1,0,0,1,2,0},
        {0,1,2,10,4,5,6,11,8,9,7,3},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };

    static constexpr Move B = Move{
        {0,1,3,7,4,5,2,6},
        {0,0,2,1,0,0,1,2},
        {0,1,9,3,4,5,10,7,8,6,2,11},
        {0,0,1,0,0,0,1,0,0,1,1,0}
    };

    // --- Create a constexpr frozen map with all 18 moves ---
    static constexpr auto move_map = frozen::make_unordered_map<char, const Move*, 6>({
        std::pair{'U', &U},
        std::pair{'R', &R},
        std::pair{'F', &F},
        std::pair{'D', &D},
        std::pair{'L', &L},
        std::pair{'B', &B}
     });

    // --- Parse a move string at compile-time ---
    static constexpr Move parse(std::string_view str) {
        // Note: frozen::unordered_map::at() throws at runtime if not found,
        // in constexpr context you must ensure key exists
        return *move_map.at(str[0]);
    }
};

inline constexpr Move operator"" _move(const char* str, std::size_t size)
{
    return Rubik::parse(std::string_view(str, size));
}