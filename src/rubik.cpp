#include "rubik.hpp"
#if defined(__SSSE3__)  // or __AVX2__, __SSE4_1__, etc.
#include <immintrin.h>

// Vectorized mod 3 for 8-bit unsigned values using lookup (0-4 safe)
inline __m128i mod3_epi8(__m128i x) {
    const __m128i table = _mm_setr_epi8(0, 1, 2, 0, 1, 2, 0, 1,
        2, 0, 1, 2, 0, 1, 2, 0);
    return _mm_shuffle_epi8(table, x);
}

void apply_move_wrong(Cube& cube, const Move& m) {
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


#include <tmmintrin.h> // For SSSE3 _mm_shuffle_epi8
#include <cstring>

__m128i mullo_epi8(__m128i a, __m128i b) {
    __m128i a_lo = _mm_unpacklo_epi8(a, _mm_setzero_si128()); // Lower 8 bytes -> 16-bit
    __m128i a_hi = _mm_unpackhi_epi8(a, _mm_setzero_si128());

    __m128i b_lo = _mm_unpacklo_epi8(b, _mm_setzero_si128());
    __m128i b_hi = _mm_unpackhi_epi8(b, _mm_setzero_si128());

    __m128i lo_mul = _mm_mullo_epi16(a_lo, b_lo);
    __m128i hi_mul = _mm_mullo_epi16(a_hi, b_hi);

    return _mm_packus_epi16(lo_mul, hi_mul); // Pack back to 8-bit
}

void apply_move(Cube& cube, const Move& m) {
    alignas(16) uint8_t shuffle_mask_corner[16] = {};
    alignas(16) uint8_t shuffle_mask_edge[16] = {};

    // Fill shuffle masks from move permutation
    for (int i = 0; i < 8; ++i)
        shuffle_mask_corner[i] = m.corner_perm[i];
    for (int i = 0; i < 12; ++i)
        shuffle_mask_edge[i] = m.edge_perm[i];

    __m128i c_perm = _mm_loadl_epi64((__m128i*)cube.corner_perm);       // 8 bytes
    __m128i c_orient = _mm_loadl_epi64((__m128i*)cube.corner_orient);   // 8 bytes
    __m128i e_perm = _mm_loadu_si128((__m128i*)cube.edge_perm);         // 12 bytes
    __m128i e_orient = _mm_loadu_si128((__m128i*)cube.edge_orient);     // 12 bytes

    __m128i mask_corner = _mm_loadu_si128((__m128i*)shuffle_mask_corner);
    __m128i mask_edge = _mm_loadu_si128((__m128i*)shuffle_mask_edge);

    __m128i c_perm_shuffled = _mm_shuffle_epi8(c_perm, mask_corner);
    __m128i c_orient_shuffled = _mm_shuffle_epi8(c_orient, mask_corner);
    __m128i e_perm_shuffled = _mm_shuffle_epi8(e_perm, mask_edge);
    __m128i e_orient_shuffled = _mm_shuffle_epi8(e_orient, mask_edge);

    __m128i delta_c = _mm_loadl_epi64((__m128i*)m.corner_orient_delta);
    __m128i delta_e = _mm_loadu_si128((__m128i*)m.edge_orient_delta);

    // Add and mod 3 for corner orientations
    __m128i c_orient_sum = _mm_add_epi8(c_orient_shuffled, delta_c);

    // Efficient mod 3 trick using SIMD (AVOID expensive divs)
    auto mod3_epi8 = [](const __m128i& v) -> __m128i {
        // x % 3 = x - (x / 3) * 3
        __m128i three = _mm_set1_epi8(3);
        __m128i div = _mm_div_epu8(v, three);   // Custom function below
        return _mm_sub_epi8(v, mullo_epi8(div, three));
    };

    // But SSE has no _mm_div_epu8! We'll do fallback
    alignas(16) uint8_t orient_sum[16];
    _mm_storeu_si128((__m128i*)orient_sum, c_orient_sum);
    for (int i = 0; i < 8; ++i)
        orient_sum[i] %= 3;
    __m128i new_c_orient = _mm_loadu_si128((__m128i*)orient_sum);

    // XOR for edge orientations
    __m128i new_e_orient = _mm_xor_si128(e_orient_shuffled, delta_e);

    // Store results
    _mm_storel_epi64((__m128i*)cube.corner_perm, c_perm_shuffled);
    _mm_storel_epi64((__m128i*)cube.corner_orient, new_c_orient);

    alignas(16) uint8_t tmp_e_perm[16];
    alignas(16) uint8_t tmp_e_orient[16];

    _mm_storeu_si128((__m128i*)tmp_e_perm, e_perm_shuffled);
    _mm_storeu_si128((__m128i*)tmp_e_orient, new_e_orient);

    std::memcpy(cube.edge_perm, tmp_e_perm, 12);
    std::memcpy(cube.edge_orient, tmp_e_orient, 12);
}
#else
void apply_move(Cube& cube, const Move& m) {
    uint8_t old_c_perm[8], old_c_orient[8];
    uint8_t old_e_perm[12], old_e_orient[12];

    std::memcpy(old_c_perm, cube.corner_perm, 8);
    std::memcpy(old_c_orient, cube.corner_orient, 8);
    std::memcpy(old_e_perm, cube.edge_perm, 12);
    std::memcpy(old_e_orient, cube.edge_orient, 12);

    for (int i = 0; i < 8; ++i) {
        uint8_t src = m.corner_perm[i];
        cube.corner_perm[i] = old_c_perm[src];
        cube.corner_orient[i] = (old_c_orient[src] + m.corner_orient_delta[i]) % 3;
    }

    for (int i = 0; i < 12; ++i) {
        uint8_t src = m.edge_perm[i];
        cube.edge_perm[i] = old_e_perm[src];
        cube.edge_orient[i] = old_e_orient[src] ^ m.edge_orient_delta[i];
    }
}
#endif



// Compose two moves: result = second followed by first (m2 � m1)
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

std::ostream& operator<<(std::ostream& s, Move m) {
    s << "corner_perm: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.corner_perm[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "corner_orient_delta: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.corner_orient_delta[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "edge_perm: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.edge_perm[i]) << (i < 11 ? ' ' : '\n');
    }

    s << "edge_orient_delta: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.edge_orient_delta[i]) << (i < 11 ? ' ' : '\n');
    }

    return s;
}

Rubik::Rubik(){
    generate_all_moves();
}

static void Rubik::generate_all_moves() {
    // Define U, R, F, B, L, D
    std::array<std::string, 6> base_names = { "U", "R", "F", "D", "L", "B" };
    std::array<Move, 6> base_moves = {
        // === U (Up face clockwise) ===
        Move {
            {3, 0, 1, 2, 4, 5, 6, 7},                // corner_perm
            {0, 0, 0, 0, 0, 0, 0, 0},                // corner_orient_delta
            {1, 2, 3, 0, 4, 5, 6, 7, 8, 9,10,11},    // edge_perm
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}     // edge_orient_delta
        },

        // === R (Right face clockwise) ===
        Move{
            {4, 1, 2, 0, 7, 5, 6, 3},                // corner_perm
            {1, 0, 0, 2, 2, 0, 0, 1},                // corner_orient_delta (twists)
            {0, 8, 2, 3, 4, 9, 6, 7, 5, 1, 10,11},   //{0, 8, 2, 3, 4, 9, 6, 7,5,1,10,11},      // edge_perm
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}     // edge_orient_delta (flip)
        },
        // === F (Front face clockwise) ===
        Move {
            {1,5,2,3,0,4,6,7},                // corner_perm
            {2,1,0,0,1,2,0,0},                // corner_orient_delta
            {11,1,2,3,8,5,6,7,0,9,10,4},      // edge_perm
            {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1}        // edge_orient_delta
        },


        // === D (Down face clockwise) ===
        Move {
            {0, 1, 2, 3, 5, 6, 7, 4},                // corner_perm
            {0, 0, 0, 0, 0, 0, 0, 0},                // corner_orient_delta
            {0, 1, 2, 3, 7, 4, 5, 6, 8, 9,10,11},    // edge_perm
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}     // edge_orient_delta
        },

        // === L (Left face clockwise) ===
        Move {
            {0, 2, 6, 3, 4, 1, 5, 7},                // corner_perm
            {0, 2, 1, 0, 0, 1, 2, 0},                // corner_orient_delta
            {0, 1, 2,10,4,5,6,11,8,9,7,3},           // edge_perm
            {0,0,0,0,0,0,0,0,0,0,0,0}             // edge_orient_delta
        },

        // === B (Back face clockwise) ===
        Move {
            {0, 1, 3,7,4,5,2,6},                // corner_perm
            {0, 0, 2, 1, 0 ,0, 1, 2},                // corner_orient_delta
            {0, 1, 9, 3,4,5,10,7,8,6,2,11},           // edge_perm
            {0, 0, 1, 0,0,0,1 ,0,0,1,1,0 }             // edge_orient_delta
        }
    };


    for (size_t i = 0; i < base_names.size(); ++i) {
        const auto& name = base_names[i];
        const auto& m = base_moves[i];

        move_map[name] = m;
        move_map[name + "2"] = compose_moves(m, m);
        move_map[name + "'"] = compose_moves(compose_moves(m, m), m);
    }
}


// Splits a std::string by the space character and returns a Move representing doing the moves successively.
Move parse_move(const std::string& move_str, const std::unordered_map<std::string, Move>& move_map) {
    if (move_str.size() == 1) return move_map.at(move_str);
    Move result = Move::identity();
    std::istringstream iss(move_str);
    std::string token;
    while (iss >> token) {
		result = compose_moves(result, move_map.at(token));
    }
	return result;
};
bool operator==(Move l, Move r) {
    return std::memcmp(&l, &r, sizeof(Move)) == 0;
}
Move get_inverse(const Move& m) {
    Move result=m;
    for (int i = 0; i < 8; i++) {
        //result.edge_orient_delta[i] = (m.edge_orient_delta[i] * 2) % 3;
        result.corner_perm[m.corner_perm[i]] = i;
    }
    for (int i = 0; i < 12; i++) {
        //result.edge_perm[i] ^= 1;
        result.edge_perm[m.edge_perm[i]] = i;
    }
    //(std::uint64_t)result.edge_orient_delta = ~(std::uint64_t)m.edge_orient_delta;



    return result;
}
