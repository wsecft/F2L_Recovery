#include "cube_view.hpp"
#include "scramble.hpp"
#include "solver.hpp"

#include <emscripten/bind.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

namespace {

std::string facelet_string(const Move& state) {
    const f2l::Facelets stickers = f2l::cube_facelets(state);
    std::string result;
    result.reserve(stickers.size());
    for (std::uint8_t sticker : stickers)
        result += static_cast<char>('0' + sticker);
    return result;
}

struct CubeInfo {
    std::string facelets;
    bool crossSolved;
    bool f2lSolved;
    int solvedBits;
};

struct SearchOutput {
    bool found;
    bool verified;
    std::string moves;
    int macroCount;
    double nodes;
    std::string finalFacelets;
    int solvedBits;
    bool crossSolved;
    bool f2lSolved;
};

struct ReconstructionOutput {
    bool found;
    std::string moves;
    int macroCount;
    double nodes;
    std::string crossFace;
    std::string inspectionMoves;
    int inspectionIndex;
    int compatibleFrameCount;
    std::string compatibleInspections;
    std::string startFacelets;
    std::string finalFacelets;
};

CubeInfo inspect_cube(const std::string& moves) {
    const Move state = Rubik::parse(moves);
    return {
        facelet_string(state),
        f2l::cross_solved(state),
        f2l::f2l_solved(state),
        static_cast<int>(f2l::solved_bits(state))
    };
}

SearchOutput search_cube(const std::string& mode,
                         const std::string& setup_moves,
                         const std::string& target_moves,
                         int slot_index, int max_depth, int timeout_ms,
                         int beam_width, bool use_pair_pruning) {
    if (max_depth < 0)
        throw std::invalid_argument("maximum depth must be non-negative");
    if (timeout_ms < 0)
        throw std::invalid_argument("timeout must be non-negative");
    if (beam_width <= 0)
        throw std::invalid_argument("beam width must be positive");

    const Move start = Rubik::parse(setup_moves);
    Move target_moves_state = Move::identity();
    if (mode == "exact" || mode == "target")
        target_moves_state = Rubik::parse(target_moves);
    f2l::SearchOptions options;
    options.max_depth = max_depth;
    options.timeout = std::chrono::milliseconds{timeout_ms};
    options.use_pair_pruning = use_pair_pruning;

    std::optional<f2l::SearchResult> result;
    f2l::Goal goal;
    if (mode == "exact") {
        goal = f2l::goal_exact(start + target_moves_state);
        result = f2l::search(start, goal, options);
    } else if (mode == "f2l") {
        goal = f2l::goal_f2l(start);
        result = f2l::search(start, goal, options);
    } else if (mode == "slot") {
        constexpr std::array<f2l::Slot, 4> slots{
            f2l::FR, f2l::FL, f2l::BL, f2l::BR
        };
        if (slot_index < 0 ||
            static_cast<std::size_t>(slot_index) >= slots.size())
            throw std::invalid_argument("invalid F2L slot");
        goal = f2l::goal_slot(start, slots[slot_index]);
        result = f2l::search(start, goal, options);
    } else if (mode == "target") {
        if (!f2l::f2l_solved(target_moves_state))
            throw std::invalid_argument(
                "target moves must describe an F2L-solved state");
        const Move canonical_target = f2l::normalise(target_moves_state);
        goal = [canonical_target](const Move& state) {
            for (int turns = 0; turns < 4; ++turns) {
                if (state == Rubik::y * turns + canonical_target) return true;
            }
            return false;
        };
        result = f2l::search_f2l_target(
            start, target_moves_state, options,
            static_cast<std::size_t>(beam_width));
    } else {
        throw std::invalid_argument("unknown search mode");
    }

    const Move final_state =
        result ? start + Rubik::parse(result->moves) : start;
    const bool verified =
        result && f2l::verify(start, result->moves, goal);
    if (result && !verified)
        throw std::runtime_error("search returned a result that failed verification");
    return {
        result.has_value(),
        verified,
        result ? result->moves : "",
        result ? result->macro_count : 0,
        result ? static_cast<double>(result->nodes) : 0.0,
        facelet_string(final_state),
        static_cast<int>(f2l::solved_bits(final_state)),
        f2l::cross_solved(final_state),
        f2l::f2l_solved(final_state)
    };
}

ReconstructionOutput reconstruct_f2l(
    const std::string& scramble, const std::string& cross,
    const std::string& last_layer, int max_depth, int timeout_ms,
    int beam_width) {
    if (max_depth < 0)
        throw std::invalid_argument("maximum depth must be non-negative");
    if (timeout_ms < 0)
        throw std::invalid_argument("timeout must be non-negative");
    if (beam_width <= 0)
        throw std::invalid_argument("beam width must be positive");

    f2l::SearchOptions options;
    options.max_depth = max_depth;
    options.timeout = std::chrono::milliseconds{timeout_ms};
    const auto result = f2l::reconstruct_unknown_f2l(
        scramble, cross, last_layer, options,
        static_cast<std::size_t>(beam_width));
    if (!result)
        return {false, "", 0, 0.0, "", "", -1, 0, "", "", ""};
    return {
        true,
        result->search.moves,
        result->search.macro_count,
        static_cast<double>(result->search.nodes),
        result->cross_face,
        result->inspection_moves,
        static_cast<int>(result->inspection_index),
        static_cast<int>(result->compatible_frame_count),
        result->compatible_inspections,
        facelet_string(result->start),
        facelet_string(result->f2l_state)
    };
}

std::string make_scramble(int count, unsigned int seed) {
    if (count > 1000)
        throw std::invalid_argument("scramble count must not exceed 1000");
    return random_scramble(count, seed);
}

} // namespace

EMSCRIPTEN_BINDINGS(f2l_web) {
    using emscripten::value_object;

    value_object<CubeInfo>("CubeInfo")
        .field("facelets", &CubeInfo::facelets)
        .field("crossSolved", &CubeInfo::crossSolved)
        .field("f2lSolved", &CubeInfo::f2lSolved)
        .field("solvedBits", &CubeInfo::solvedBits);
    value_object<SearchOutput>("SearchOutput")
        .field("found", &SearchOutput::found)
        .field("verified", &SearchOutput::verified)
        .field("moves", &SearchOutput::moves)
        .field("macroCount", &SearchOutput::macroCount)
        .field("nodes", &SearchOutput::nodes)
        .field("finalFacelets", &SearchOutput::finalFacelets)
        .field("solvedBits", &SearchOutput::solvedBits)
        .field("crossSolved", &SearchOutput::crossSolved)
        .field("f2lSolved", &SearchOutput::f2lSolved);
    value_object<ReconstructionOutput>("ReconstructionOutput")
        .field("found", &ReconstructionOutput::found)
        .field("moves", &ReconstructionOutput::moves)
        .field("macroCount", &ReconstructionOutput::macroCount)
        .field("nodes", &ReconstructionOutput::nodes)
        .field("crossFace", &ReconstructionOutput::crossFace)
        .field("inspectionMoves", &ReconstructionOutput::inspectionMoves)
        .field("inspectionIndex", &ReconstructionOutput::inspectionIndex)
        .field("compatibleFrameCount", &ReconstructionOutput::compatibleFrameCount)
        .field("compatibleInspections", &ReconstructionOutput::compatibleInspections)
        .field("startFacelets", &ReconstructionOutput::startFacelets)
        .field("finalFacelets", &ReconstructionOutput::finalFacelets);

    emscripten::function("inspectCube", &inspect_cube);
    emscripten::function("searchCube", &search_cube);
    emscripten::function("reconstructF2L", &reconstruct_f2l);
    emscripten::function("randomScramble", &make_scramble);
}
