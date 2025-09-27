#pragma once
#include <array>
#include <string_view>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>



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
    uint8_t corner_perm[8]{};
    uint8_t corner_orient_delta[8]{};
    uint8_t edge_perm[12]{};
    uint8_t edge_orient_delta[12]{};
    constexpr static Move identity() {
        Move m;
        for (int i = 0; i < 8; i++) {
            m.corner_perm[i] = i;
            m.corner_orient_delta[i] = 0;
        }
        for (int i = 0; i < 12; i++) {
            m.edge_perm[i] = i;
            m.edge_orient_delta[i] = 0;
        }
        return m;
    }

};

class Rubik {
    static std::unordered_map<std::string_view, Move> move_map;
    constexpr static void generate_all_moves();
public:
    constexpr static Move parse(std::string_view);
};

constexpr std::ostream& operator<<(std::ostream&, Move m);
constexpr std::ostream& operator<<(std::ostream&, Cube m);
constexpr bool operator==(Move l, Move r);
constexpr Move operator ""_move(const char*, std::size_t);
constexpr void apply_move(Cube& cube, const Move& m);
constexpr Move operator+(const Move& m1, const Move& m2);
constexpr Move operator*(const Move&, const int);
constexpr Move operator-(const Move& m);