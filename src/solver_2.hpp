#include "rubik.hpp"
#include <bit>
#include <frozen/unordered_set.h>

// specialization for Move
template <> struct frozen::elsa<Move> {
    constexpr std::size_t operator()(Move const& m, std::size_t seed) const {
        auto mix = [](std::size_t h, uint8_t v) constexpr {
            h ^= static_cast<std::size_t>(v) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            return h;
            };

        std::size_t h = seed;

        for (uint8_t v : m.cp)      h = mix(h, v);
        for (uint8_t v : m.co)      h = mix(h, v);
        for (uint8_t v : m.ep)      h = mix(h, v);
        for (uint8_t v : m.eo)      h = mix(h, v);
        for (uint8_t v : m.centers) h = mix(h, v);

        return h;
    }
};

template<int N>
consteval auto dedup(std::array<Move, N> arr) {
    std::array<Move, N> out{};
    int out_size = 0;

    for (int i = 0; i < N; ++i) {
        bool exists = false;
        for (int j = 0; j < out_size; ++j) {
            if (arr[i] == out[j]) { exists = true; break; }
        }
        if (!exists) {
            out[out_size++] = arr[i];
        }
    }

    // Shrink to exact size
    std::array<Move, out_size> final{};
    for (int i = 0; i < out_size; ++i) final[i] = out[i];
    return final;
}

template<int N> std::array<Move, 4*N> consteval generateWithDoubles(std::array<std::string_view, N> base) {
    std::array<Move, 4 * N> ret{};
    for (int i = 0; i < N; i++) {
        Move m = Rubik::parse(base[i]);
        ret[4 * i + 0] = m;
        ret[4 * i + 1] = -m;
        ret[4 * i + 2] = m * 2;
        ret[4 * i + 3] = m + "y2"_move;
    }
    return ret;
}

template<int N>
consteval auto generate_set(std::array<std::string_view, N> base) {
    constexpr auto arr = generateWithDoubles<N>(base);
    constexpr auto unique_arr = dedup(arr);
    return unique_arr;
}

constexpr auto base_moves = std::to_array<std::string_view>({
"U", "y", "R U R'", "R' U' R"
    });
static constexpr auto moves = generate_set<base_moves.size()>(base_moves);

class F2LPartialSolver {
public:
    Move findCandidateF2L(Move startAlg, Move endAlg) {
        Move F2L = endAlg+ -startAlg;

    }
};