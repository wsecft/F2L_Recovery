#pragma once
#include <array>
#include <string_view>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>
struct Cube {
    uint8_t corner_perm[8];   // permutation of 8 corners (values 0-7)
    uint8_t corner_orient[8]; // orientation of each corner (values 0-2)
    uint8_t edge_perm[12];    // permutation of 12 edges (values 0-11)
    uint8_t edge_orient[12];  // orientation of each edge (values 0-1)
    static Cube identity() {
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
};

struct Move {
    uint8_t corner_perm[8]{};
    uint8_t corner_orient_delta[8]{};
    uint8_t edge_perm[12]{};
    uint8_t edge_orient_delta[12]{};
};

void apply_move(Cube& cube, const Move& m);
Move compose_moves(const Move& m1, const Move& m2);
std::unordered_map<std::string, Move> generate_all_moves();