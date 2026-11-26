#pragma once

#include "rubik.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace detail {

inline constexpr std::array<std::string_view, 16> triggers = {
    "R U R'", "R U' R'", "R U2 R'", "R' F R F'",
    "R' U R", "R' U' R", "R' U2 R", "F R' F' R",
    "L' U' L", "L U L'", "L' U L", "L U' L'",
    "L' U' L U", "L U L' U'", "L' U2 L", "L U2 L'"
};

inline constexpr std::array<std::string_view, 7> transitions = {
    "U", "U'", "U2", "", "y", "y'", "y2"
};

consteval auto makeCombined()
{
    std::array<Move, triggers.size() * transitions.size()> arr{};
    for (std::size_t ti = 0; ti < triggers.size(); ++ti) {
        for (std::size_t ui = 0; ui < transitions.size(); ++ui) {
            arr[ti * transitions.size() + ui] =
                Rubik::parse(triggers[ti]) + Rubik::parse(transitions[ui]);
        }
    }
    return arr;
}

consteval auto makeInvCombined()
{
    std::array<Move, triggers.size() * transitions.size()> arr{};
    for (std::size_t ti = 0; ti < triggers.size(); ++ti) {
        for (std::size_t ui = 0; ui < transitions.size(); ++ui) {
            arr[ti * transitions.size() + ui] =
                -Rubik::parse(transitions[ui]) + -Rubik::parse(triggers[ti]);
        }
    }
    return arr;
}

inline constexpr auto combined = makeCombined();
inline constexpr auto invCombined = makeInvCombined();

} // namespace detail

static_assert(detail::triggers.size() == 16);
static_assert(detail::transitions.size() == 7);

namespace f2l {

struct Slot {
    std::uint8_t corner;
    std::uint8_t edge;
};

inline constexpr Slot FR{4, 8};
inline constexpr Slot FL{5, 11};
inline constexpr Slot BL{6, 10};
inline constexpr Slot BR{7, 9};

inline constexpr std::array<std::uint8_t, 4> cross_edges{4, 5, 6, 7};

inline constexpr std::array<Move, 24> rotations = [] {
    std::array<Move, 24> out{};
    out[0] = Move::identity();
    std::size_t count = 1;
    for (std::size_t i = 0; i < count && count < out.size(); ++i) {
        const Move bases[] = {Rubik::x, Rubik::y, Rubik::z};
        for (const Move& base : bases) {
            for (int power = 1; power <= 3; ++power) {
                Move candidate = out[i] + base * power;
                bool exists = false;
                for (std::size_t j = 0; j < count; ++j) {
                    if (out[j] == candidate) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) out[count++] = candidate;
            }
        }
    }
    return out;
}();

inline Move normalise(const Move& state) {
    Move inverse_centers = Move::identity();
    for (std::uint8_t i = 0; i < 6; ++i) {
        inverse_centers.centers[state.centers[i]] = i;
    }
    for (const Move& rotation : rotations) {
        if (rotation.centers == inverse_centers.centers)
            return state + rotation;
    }
    return state;
}

inline bool cross_solved(const Move& state) {
    Move normal = normalise(state);
    for (std::uint8_t position : cross_edges) {
        if (normal.ep[position] != position || normal.eo[position] != 0)
            return false;
    }
    return true;
}

inline bool slot_solved(const Move& state, Slot slot) {
    Move normal = normalise(state);
    return normal.cp[slot.corner] == slot.corner &&
           normal.co[slot.corner] == 0 &&
           normal.ep[slot.edge] == slot.edge &&
           normal.eo[slot.edge] == 0;
}

inline bool f2l_solved(const Move& state) {
    if (!cross_solved(state)) return false;
    return slot_solved(state, FR) && slot_solved(state, FL) &&
           slot_solved(state, BL) && slot_solved(state, BR);
}

inline std::uint16_t solved_bits(const Move& state) {
    Move normal = normalise(state);
    std::uint16_t bits = 0;
    for (std::size_t i = 0; i < cross_edges.size(); ++i) {
        std::uint8_t position = cross_edges[i];
        if (normal.ep[position] == position && normal.eo[position] == 0)
            bits |= static_cast<std::uint16_t>(1u << i);
    }
    const Slot slots[] = {FR, FL, BL, BR};
    for (std::size_t i = 0; i < 4; ++i) {
        if (slot_solved(normal, slots[i]))
            bits |= static_cast<std::uint16_t>(1u << (i + 4));
    }
    return bits;
}

struct SearchOptions {
    int max_depth = 4;
    std::chrono::milliseconds timeout{5000};
    std::stop_token stop{};
};

struct SearchResult {
    std::string moves;
    int macro_count = 0;
    std::uint64_t nodes = 0;
    std::vector<std::size_t> path;
};

using Goal = std::function<bool(const Move&)>;

inline Goal goal_exact(const Move& target) {
    return [target](const Move& state) { return state == target; };
}

inline Goal goal_slot(const Move& start, Slot target) {
    std::uint16_t required = solved_bits(normalise(start));
    const Slot slots[] = {FR, FL, BL, BR};
    for (std::size_t i = 0; i < 4; ++i) {
        if (slots[i].corner == target.corner && slots[i].edge == target.edge)
            required |= static_cast<std::uint16_t>(1u << (i + 4));
    }
    return [required](const Move& state) {
        return (solved_bits(state) & required) == required;
    };
}

inline Goal goal_f2l(const Move& start) {
    std::uint16_t preserved = solved_bits(normalise(start));
    std::uint16_t required = preserved | 0xffu;
    return [required](const Move& state) {
        return (solved_bits(state) & required) == required;
    };
}

inline std::string render(std::vector<std::size_t> const& path) {
    std::string result;
    for (std::size_t k : path) {
        std::size_t trigger = k / detail::transitions.size();
        std::size_t transition = k % detail::transitions.size();
        if (!result.empty()) result += ' ';
        result += detail::triggers[trigger];
        if (!detail::transitions[transition].empty()) {
            result += ' ';
            result += detail::transitions[transition];
        }
    }
    return result;
}

template <class GoalCallable>
bool verify(const Move& start, std::string_view moves, GoalCallable goal) {
    try {
        Move state = start + Rubik::parse(moves);
        return goal(state);
    } catch (...) {
        return false;
    }
}

template <class GoalCallable>
std::optional<SearchResult> search(const Move& start, GoalCallable goal,
                                   const SearchOptions& options = {}) {
    if (goal(start)) {
        return SearchResult{"", 0, 0};
    }

    std::unordered_map<Move, std::int8_t> transposition;
    std::uint64_t nodes = 0;
    const auto deadline = std::chrono::steady_clock::now() + options.timeout;

    for (int depth = 1; depth <= options.max_depth; ++depth) {
        transposition.clear();
        std::vector<std::size_t> path;
        bool stopped = false;
        bool found = false;
        SearchResult result;

        auto dfs = [&](auto&& self, const Move& state, int remaining,
                       std::size_t previous) -> bool {
            ++nodes;
            if ((nodes & 1023u) == 0) {
                if (options.stop.stop_requested() ||
                    std::chrono::steady_clock::now() >= deadline) {
                    stopped = true;
                    return false;
                }
            }
            if (goal(state)) {
                result.moves = render(path);
                result.macro_count = static_cast<int>(path.size());
                result.nodes = nodes;
                result.path = path;
                found = true;
                return true;
            }
            if (remaining == 0) return false;

            const int previous_remaining =
                transposition.contains(state) ? transposition[state] : -1;
            if (previous_remaining >= remaining) return false;
            transposition[state] = static_cast<std::int8_t>(remaining);

            for (std::size_t k = 0;
                 k < detail::combined.size() && !stopped && !found; ++k) {
                if (path.size() > 0 &&
                    detail::combined[k] == detail::invCombined[previous])
                    continue;

                Move next = state + detail::combined[k];
                path.push_back(k);
                if (self(self, next, remaining - 1, k)) return true;
                path.pop_back();
            }
            return false;
        };

        if (dfs(dfs, start, depth, 0)) return result;
        if (stopped) break;
    }
    return std::nullopt;
}

} // namespace f2l

class F2LPartialSolver {
public:
    inline static constexpr const auto& combined = detail::combined;
    inline static constexpr const auto& invCombined = detail::invCombined;

    static std::vector<int> findCandidateF2L_(const Move& start,
                                               const Move& end, int depth) {
        auto result = f2l::search(
            start, f2l::goal_exact(end), f2l::SearchOptions{depth});
        if (!result) return {};
        std::vector<int> path;
        path.reserve(result->path.size());
        for (std::size_t k : result->path)
            path.push_back(static_cast<int>(k));
        return path;
    }

    static std::string findCandidateF2L(const Move& start,
                                        const Move& end, int depth) {
        auto result = f2l::search(
            start, f2l::goal_exact(end), f2l::SearchOptions{depth});
        if (!result) return {};
        return result->moves;
    }
};
