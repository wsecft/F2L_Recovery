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

class Rubik {
    static std::unordered_map<std::string,Move> move_map;
    static void generate_all_moves();
public:
    static Move parse()
}

struct Move {
    uint8_t corner_perm[8]{};
    uint8_t corner_orient_delta[8]{};
    uint8_t edge_perm[12]{};
    uint8_t edge_orient_delta[12]{};
    static Move identity() {
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

std::ostream& operator<<(std::ostream&, Move m);
bool operator==(Move l, Move r);
void apply_move(Cube& cube, const Move& m);
void apply_move_SIMD(Cube& cube, const Move& m);
Move compose_moves(const Move& m1, const Move& m2);
std::unordered_map<std::string, Move> generate_all_moves();
Move parse_move(const std::string& move_str, const std::unordered_map<std::string, Move>& move_map);
Move get_inverse(const Move& m);