#pragma once

#include "rubik.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <deque>
#include <limits>
#include <optional>
#include <stop_token>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
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
    bool use_pair_pruning = false;
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

inline std::uint16_t pair_key(const Move& state, Slot slot) {
    const std::uint16_t corner =
        static_cast<std::uint16_t>(state.cp[slot.corner] * 3u +
                                   state.co[slot.corner]);
    const std::uint16_t edge =
        static_cast<std::uint16_t>(state.ep[slot.edge] * 2u +
                                   state.eo[slot.edge]);
    return static_cast<std::uint16_t>(corner * 24u + edge);
}

inline const std::array<std::uint8_t, 576>& pair_distance_table(Slot slot) {
    static const std::array<std::array<std::uint8_t, 576>, 4> tables = [] {
        std::array<std::array<std::uint8_t, 576>, 4> result{};
        const Slot slots[] = {FR, FL, BL, BR};
        for (std::size_t si = 0; si < 4; ++si) {
            result[si].fill(std::numeric_limits<std::uint8_t>::max());
            Move current = Move::identity();
            std::deque<Move> queue;
            const std::uint16_t target =
                static_cast<std::uint16_t>(slots[si].corner * 3u * 24u +
                                           slots[si].edge * 2u);
            result[si][target] = 0;
            queue.push_back(current);
            while (!queue.empty()) {
                current = queue.front();
                queue.pop_front();
                std::uint16_t key = pair_key(current, slots[si]);
                std::uint8_t distance = result[si][key];
                for (const Move& macro : detail::combined) {
                    Move next = current + macro;
                    std::uint16_t next_key = pair_key(next, slots[si]);
                    if (result[si][next_key] != std::numeric_limits<std::uint8_t>::max())
                        continue;
                    result[si][next_key] =
                        static_cast<std::uint8_t>(distance + 1u);
                    queue.push_back(next);
                }
            }
        }
        return result;
    }();

    if (slot.corner == FR.corner && slot.edge == FR.edge) return tables[0];
    if (slot.corner == FL.corner && slot.edge == FL.edge) return tables[1];
    if (slot.corner == BL.corner && slot.edge == BL.edge) return tables[2];
    return tables[3];
}

inline std::uint8_t pair_distance(const Move& state, Slot slot) {
    const std::uint16_t key = pair_key(state, slot);
    return pair_distance_table(slot)[key];
}

inline std::uint8_t pair_lower_bound(const Move& state) {
    const Slot slots[] = {FR, FL, BL, BR};
    std::uint8_t result = 0;
    for (Slot slot : slots) {
        std::uint8_t distance = pair_distance(state, slot);
        if (distance > result) result = distance;
    }
    return result;
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
            if (options.use_pair_pruning &&
                remaining < pair_lower_bound(state)) {
                return false;
            }

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

inline std::optional<SearchResult> search_f2l_target(
    const Move& start, const Move& target,
    const SearchOptions& options = {}, std::size_t beam_width = 5000) {
    if (options.max_depth < 0 || beam_width == 0) return std::nullopt;
    const auto deadline = std::chrono::steady_clock::now() + options.timeout;

    const Move canonical_target = normalise(target);
    if (!f2l_solved(canonical_target)) return std::nullopt;

    std::array<Move, 4> targets{};
    std::array<Move, 4> inverse_targets{};
    for (std::size_t i = 0; i < targets.size(); ++i) {
        targets[i] = Rubik::y * static_cast<int>(i) + canonical_target;
        inverse_targets[i] = -targets[i];
        if (start == targets[i])
            return SearchResult{"", 0, 0, {}};
    }

    struct ProjectionTable {
        std::vector<std::uint8_t> distances;
        std::uint32_t position_radix;
        std::uint32_t orientation_radix;
        std::uint32_t orientation_count;
        std::uint8_t first_piece;
        bool edges;

        std::uint32_t key(const Move& state) const {
            std::array<std::uint8_t, 4> positions{};
            std::array<std::uint8_t, 4> orientations{};
            const std::uint8_t count = edges ? 12 : 8;
            for (std::uint8_t position = 0; position < count; ++position) {
                const std::uint8_t piece =
                    edges ? state.ep[position] : state.cp[position];
                if (piece < first_piece || piece >= first_piece + 4)
                    continue;
                const std::size_t index = piece - first_piece;
                positions[index] = position;
                orientations[index] =
                    edges ? state.eo[position] : state.co[position];
            }

            std::uint32_t position_key = 0;
            std::uint32_t position_factor = 1;
            std::uint32_t orientation_key = 0;
            std::uint32_t orientation_factor = 1;
            for (std::size_t i = 0; i < positions.size(); ++i) {
                position_key += positions[i] * position_factor;
                position_factor *= position_radix;
                orientation_key += orientations[i] * orientation_factor;
                orientation_factor *= orientation_radix;
            }
            return position_key * orientation_count + orientation_key;
        }
    };

    struct HeuristicTables {
        std::array<std::array<std::uint8_t, 576>, 4> pairs;
        std::array<ProjectionTable, 4> pieces;
    };

    static const HeuristicTables tables = [] {
        HeuristicTables result{};
        const Slot slots[] = {FR, FL, BL, BR};

        for (std::size_t si = 0; si < 4; ++si) {
            auto& distances = result.pairs[si];
            distances.fill(std::numeric_limits<std::uint8_t>::max());
            Move identity = Move::identity();
            distances[pair_key(identity, slots[si])] = 0;
            std::deque<Move> queue{identity};

            while (!queue.empty()) {
                Move current = queue.front();
                queue.pop_front();
                const std::uint8_t next_distance =
                    static_cast<std::uint8_t>(
                        distances[pair_key(current, slots[si])] + 1u);
                for (const Move& inverse_macro : detail::invCombined) {
                    Move predecessor = current + inverse_macro;
                    const std::uint16_t key = pair_key(predecessor, slots[si]);
                    if (distances[key] !=
                        std::numeric_limits<std::uint8_t>::max())
                        continue;
                    distances[key] = next_distance;
                    queue.push_back(predecessor);
                }
            }
        }

        auto make_projection = [](bool edges, std::uint8_t first_piece) {
            ProjectionTable table{
                {},
                edges ? 12u : 8u,
                edges ? 2u : 3u,
                edges ? 16u : 81u,
                first_piece,
                edges
            };
            const std::uint32_t position_count =
                table.position_radix * table.position_radix *
                table.position_radix * table.position_radix;
            table.distances.assign(
                position_count * table.orientation_count,
                std::numeric_limits<std::uint8_t>::max());

            std::array<std::uint8_t, 4> positions{};
            std::array<std::uint8_t, 4> orientations{};
            for (std::size_t i = 0; i < positions.size(); ++i)
                positions[i] = static_cast<std::uint8_t>(first_piece + i);

            const auto encode = [&](const auto& pos, const auto& ori) {
                std::uint32_t position_key = 0;
                std::uint32_t position_factor = 1;
                std::uint32_t orientation_key = 0;
                std::uint32_t orientation_factor = 1;
                for (std::size_t i = 0; i < pos.size(); ++i) {
                    position_key += pos[i] * position_factor;
                    position_factor *= table.position_radix;
                    orientation_key += ori[i] * orientation_factor;
                    orientation_factor *= table.orientation_radix;
                }
                return position_key * table.orientation_count +
                       orientation_key;
            };
            const auto decode = [](std::uint32_t value, std::uint32_t radix) {
                std::array<std::uint8_t, 4> decoded{};
                for (std::uint8_t& digit : decoded) {
                    digit = static_cast<std::uint8_t>(value % radix);
                    value /= radix;
                }
                return decoded;
            };

            const std::uint32_t root = encode(positions, orientations);
            table.distances[root] = 0;
            std::deque<std::uint32_t> queue{root};
            while (!queue.empty()) {
                const std::uint32_t current_key = queue.front();
                queue.pop_front();
                const std::uint32_t orientation_key =
                    current_key % table.orientation_count;
                auto current_positions =
                    decode(current_key / table.orientation_count,
                           table.position_radix);
                auto current_orientations =
                    decode(orientation_key, table.orientation_radix);
                const std::uint8_t next_distance =
                    static_cast<std::uint8_t>(
                        table.distances[current_key] + 1u);

                for (std::size_t k = 0; k < detail::combined.size(); ++k) {
                    auto next_positions = current_positions;
                    auto next_orientations = current_orientations;
                    for (std::size_t i = 0; i < next_positions.size(); ++i) {
                        const std::uint8_t destination = edges
                            ? detail::combined[k].ep[current_positions[i]]
                            : detail::combined[k].cp[current_positions[i]];
                        next_positions[i] = destination;
                        if (edges) {
                            next_orientations[i] ^=
                                detail::invCombined[k].eo[destination];
                        } else {
                            next_orientations[i] = static_cast<std::uint8_t>(
                                (next_orientations[i] +
                                 detail::invCombined[k].co[destination]) % 3u);
                        }
                    }

                    const std::uint32_t next_key =
                        encode(next_positions, next_orientations);
                    if (table.distances[next_key] !=
                        std::numeric_limits<std::uint8_t>::max())
                        continue;
                    table.distances[next_key] = next_distance;
                    queue.push_back(next_key);
                }
            }
            return table;
        };

        result.pieces = {
            make_projection(false, 0),
            make_projection(true, 0),
            make_projection(false, 4),
            make_projection(true, 4)
        };
        return result;
    }();

    const Slot slots[] = {FR, FL, BL, BR};
    auto goal_index = [&](const Move& state) -> std::size_t {
        for (std::size_t i = 0; i < targets.size(); ++i)
            if (state.centers == targets[i].centers) return i;
        return targets.size();
    };
    auto heuristic = [&](const Move& state, std::size_t target_index) {
        const Move relative = inverse_targets[target_index] + state;
        std::uint8_t maximum = 0;
        std::uint32_t total = 0;

        for (std::size_t i = 0; i < 4; ++i) {
            const std::uint8_t distance =
                tables.pairs[i][pair_key(relative, slots[i])];
            maximum = std::max(maximum, distance);
            total += distance;
        }
        for (const ProjectionTable& table : tables.pieces) {
            const std::uint8_t distance =
                table.distances[table.key(relative)];
            maximum = std::max(maximum, distance);
            total += distance;
        }
        return std::pair{maximum, total};
    };

    struct BeamNode {
        Move state;
        std::uint32_t parent;
        std::uint16_t macro;
    };
    struct Candidate {
        Move state;
        std::uint32_t parent;
        std::uint16_t macro;
        std::uint8_t lower_bound;
        std::uint32_t total_distance;
    };

    std::vector<BeamNode> nodes{{start, 0, 0}};
    std::vector<std::uint32_t> frontier{0};
    std::unordered_set<Move> visited{start};
    std::uint64_t expanded = 0;
    if (options.stop.stop_requested() ||
        std::chrono::steady_clock::now() >= deadline)
        return std::nullopt;

    for (int depth = 1; depth <= options.max_depth; ++depth) {
        std::vector<Candidate> candidates;
        candidates.reserve(frontier.size() * detail::combined.size());
        bool stopped = false;

        for (std::uint32_t parent : frontier) {
            for (std::size_t k = 0; k < detail::combined.size(); ++k) {
                if ((++expanded & 1023u) == 0 &&
                    (options.stop.stop_requested() ||
                     std::chrono::steady_clock::now() >= deadline)) {
                    stopped = true;
                    break;
                }
                if (parent != 0 &&
                    detail::combined[k] ==
                        detail::invCombined[nodes[parent].macro])
                    continue;

                Move next = nodes[parent].state + detail::combined[k];
                const std::size_t target_index = goal_index(next);
                if (target_index < targets.size() &&
                    next == targets[target_index]) {
                    std::vector<std::size_t> path;
                    for (std::uint32_t at = parent; at != 0;
                         at = nodes[at].parent)
                        path.push_back(nodes[at].macro);
                    std::reverse(path.begin(), path.end());
                    path.push_back(k);
                    return SearchResult{
                        render(path), static_cast<int>(path.size()),
                        expanded, std::move(path)
                    };
                }
                if (visited.contains(next) ||
                    target_index == targets.size())
                    continue;

                const auto [lower_bound, total_distance] =
                    heuristic(next, target_index);
                if (lower_bound ==
                        std::numeric_limits<std::uint8_t>::max() ||
                    lower_bound > options.max_depth - depth)
                    continue;
                candidates.push_back({
                    std::move(next), parent, static_cast<std::uint16_t>(k),
                    lower_bound, total_distance
                });
            }
            if (stopped) break;
        }
        if (stopped || candidates.empty()) return std::nullopt;

        std::stable_sort(candidates.begin(), candidates.end(),
                         [](const Candidate& left, const Candidate& right) {
            if (left.lower_bound != right.lower_bound)
                return left.lower_bound < right.lower_bound;
            return left.total_distance < right.total_distance;
        });

        std::unordered_set<Move> selected;
        selected.reserve(beam_width);
        std::vector<std::uint32_t> next_frontier;
        next_frontier.reserve(beam_width);
        for (Candidate& candidate : candidates) {
            if (visited.contains(candidate.state) ||
                !selected.insert(candidate.state).second)
                continue;
            const std::uint32_t index =
                static_cast<std::uint32_t>(nodes.size());
            visited.insert(candidate.state);
            nodes.push_back({
                std::move(candidate.state), candidate.parent, candidate.macro
            });
            next_frontier.push_back(index);
            if (next_frontier.size() == beam_width) break;
        }
        if (next_frontier.empty()) return std::nullopt;
        frontier = std::move(next_frontier);
    }
    return std::nullopt;
}

inline std::optional<SearchResult> recover(const Move& start, const Move& end,
                                           const SearchOptions& options = {}) {
    return search(start, goal_exact(end), options);
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
