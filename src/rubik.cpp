#pragma once
#include <immintrin.h>
#include "rubik.hpp"

// Vectorized mod 3 for 8-bit unsigned values using lookup (0-4 safe)
inline __m128i mod3_epi8(__m128i x) {
    const __m128i table = _mm_setr_epi8(0, 1, 2, 0, 1, 2, 0, 1,
        2, 0, 1, 2, 0, 1, 2, 0);
    return _mm_shuffle_epi8(table, x);
}

void apply_move(Cube& cube, const Move& m) {
    // Load data into SIMD registers
    __m128i c_perm = _mm_loadl_epi64((__m128i*)cube.corner_perm);      // 8 bytes
    __m128i c_orient = _mm_loadl_epi64((__m128i*)cube.corner_orient);  // 8 bytes
    __m128i e_perm = _mm_loadu_si128((__m128i*)cube.edge_perm);        // 12 bytes (loaded as 16)
    __m128i e_orient = _mm_loadu_si128((__m128i*)cube.edge_orient);    // 12 bytes (loaded as 16)

    // Permutation indices
    __m128i c_idx = _mm_loadl_epi64((__m128i*)m.corner_perm);
    __m128i e_idx = _mm_loadu_si128((__m128i*)m.edge_perm);

    // Shuffle permutations
    __m128i new_c_perm = _mm_shuffle_epi8(c_perm, c_idx);
    __m128i new_e_perm = _mm_shuffle_epi8(e_perm, e_idx);

    // Orientation deltas
    __m128i c_delta = _mm_loadl_epi64((__m128i*)m.corner_orient_delta);
    __m128i e_delta = _mm_loadu_si128((__m128i*)m.edge_orient_delta);

    // Shuffle orientation bases
    __m128i base_c_orient = _mm_shuffle_epi8(c_orient, c_idx);
    __m128i base_e_orient = _mm_shuffle_epi8(e_orient, e_idx);

    // Add corner orientations and mod 3
    __m128i corner_or_sum = _mm_add_epi8(base_c_orient, c_delta);
    __m128i new_c_orient = mod3_epi8(corner_or_sum);

    // XOR edge orientations
    __m128i new_e_orient = _mm_xor_si128(base_e_orient, e_delta);

    // Store corners (safe: 8 bytes)
    _mm_storel_epi64((__m128i*)cube.corner_perm, new_c_perm);
    _mm_storel_epi64((__m128i*)cube.corner_orient, new_c_orient);

    // Store edges using temporary buffer
    alignas(16) uint8_t tmp_edge_perm[16];
    alignas(16) uint8_t tmp_edge_orient[16];

    _mm_storeu_si128((__m128i*)tmp_edge_perm, new_e_perm);
    _mm_storeu_si128((__m128i*)tmp_edge_orient, new_e_orient);

    std::memcpy(cube.edge_perm, tmp_edge_perm, 12);
    std::memcpy(cube.edge_orient, tmp_edge_orient, 12);
}


// Compose two moves: result = second followed by first (m2 • m1)
Move compose_moves(const Move& a, const Move& b) {
    Move result{};

    for (int i = 0; i < 8; ++i) {
        result.corner_perm[i] = a.corner_perm[b.corner_perm[i]];
        result.corner_orient_delta[i] =
            (b.corner_orient_delta[i] + a.corner_orient_delta[b.corner_perm[i]]) % 3;
    }

    for (int i = 0; i < 12; ++i) {
        result.edge_perm[i] = a.edge_perm[b.edge_perm[i]];
        result.edge_orient_delta[i] =
            b.edge_orient_delta[i] ^ a.edge_orient_delta[b.edge_perm[i]];
    }

    return result;
}

std::unordered_map<std::string, Move> generate_all_moves() {
    std::unordered_map<std::string, Move> moves;

    // Define U, R, F, B, L, D
    std::array<std::string, 6> base_names = { "U", "R", "F", "B", "L", "D" };
    std::array<Move, 6> base_moves = {
        // U
        Move{ {3,0,1,2,4,5,6,7}, {0,0,0,0,0,0,0,0},
              {3,0,1,2,4,5,6,7,8,9,10,11}, {0,0,0,0,0,0,0,0,0,0,0,0} },

              // R
              Move{ {0,2,6,3,4,1,5,7}, {0,2,1,0,0,1,2,0},
                    {0,9,2,3,4,5,10,6,8,1,7,11}, {0,1,0,0,0,0,1,1,0,1,1,0} },

                    // F
                    Move{ {1,5,2,3,0,4,6,7}, {1,2,0,0,2,1,0,0},
                          {0,1,10,3,8,5,6,2,4,9,7,11}, {0,0,1,0,1,0,0,1,1,0,1,0} },

                          // B
                          Move{ {0,1,3,7,4,5,2,6}, {0,0,1,2,0,0,2,1},
                                {0,1,2,11,4,9,6,7,8,5,10,3}, {0,0,0,1,0,1,0,0,0,1,1,1} },

                                // L
                                Move{ {4,1,2,0,7,5,6,3}, {2,0,0,1,1,0,0,2},
                                      {4,1,2,3,8,5,6,0,11,9,10,7}, {1,0,0,0,1,0,0,1,1,0,0,1} },

                                      // D
                                      Move{ {0,1,2,3,5,6,7,4}, {0,0,0,0,0,0,0,0},
                                            {0,1,2,3,5,6,7,4,8,9,10,11}, {0,0,0,0,0,0,0,0,0,0,0,0} }
    };

    for (size_t i = 0; i < base_names.size(); ++i) {
        const auto& name = base_names[i];
        const auto& m = base_moves[i];

        moves[name] = m;
        moves[name + "2"] = compose_moves(m, m);
        moves[name + "'"] = compose_moves(compose_moves(m, m), m);
    }

    return moves;
}