#include "real_solve_fixtures.hpp"
#include "solver.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <exception>

static int failures = 0;
#define CHECK(value) do { \
    if (!(value)) { \
        std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #value); \
        ++failures; \
    } \
} while (false)

int main() {
    const Move identity = Move::identity();

    for (const RealSolveFixture& fixture : real_solve_fixtures) {
        try {
            Move state = Rubik::parse(fixture.setup);
            std::size_t replayed_phases = 0;
            std::printf("fixture %.*s %.*s (%.*s)\n",
                        static_cast<int>(fixture.id.size()), fixture.id.data(),
                        static_cast<int>(fixture.name.size()), fixture.name.data(),
                        static_cast<int>(fixture.source_url.size()),
                        fixture.source_url.data());

            for (const RealSolvePhase& phase : fixture.phases) {
                if (phase.moves.empty()) continue;
                state = state + Rubik::parse(phase.moves);
                ++replayed_phases;
            }

            CHECK(replayed_phases >= 2);
            CHECK(f2l::normalise(state) == identity);
        } catch (const std::exception& error) {
            std::printf("FAIL fixture %.*s: %s\n",
                        static_cast<int>(fixture.id.size()), fixture.id.data(),
                        error.what());
            ++failures;
        }
    }

    try {
        const RealSolveFixture& reco = real_solve_fixtures[1];
        Move pair_start = Rubik::parse(reco.setup);
        for (std::size_t i = 0; i < 3; ++i)
            pair_start = pair_start + Rubik::parse(reco.phases[i].moves);

        const Move recorded_pair_end =
            pair_start + Rubik::parse(reco.phases[3].moves);
        const std::uint16_t before = f2l::solved_bits(pair_start);
        const std::uint16_t after = f2l::solved_bits(recorded_pair_end);
        const std::uint16_t newly_solved_pairs =
            static_cast<std::uint16_t>((after & ~before) & 0x00f0u);
        CHECK(newly_solved_pairs == 0x0080u);

        f2l::SearchOptions options;
        options.max_depth = 3;
        options.timeout = std::chrono::seconds{10};
        const f2l::Goal goal = f2l::goal_slot(pair_start, f2l::BR);
        const auto result = f2l::search(pair_start, goal, options);
        CHECK(result.has_value());
        if (result) {
            CHECK(result->macro_count <= options.max_depth);
            CHECK(f2l::verify(pair_start, result->moves, goal));
        }
    } catch (const std::exception& error) {
        std::printf("FAIL reco.nz pair search: %s\n", error.what());
        ++failures;
    }

    try {
        const UnknownF2LFixture& fixture = unknown_f2l_fixture;
        const Move last_layer = Rubik::parse(fixture.last_layer);
        f2l::SearchOptions options;
        options.max_depth = 16;
        options.timeout = std::chrono::seconds{30};
        const auto result = f2l::reconstruct_unknown_f2l(
            fixture.scramble, fixture.cross, fixture.last_layer, options, 500);
        CHECK(result.has_value());
        if (result) {
            CHECK(result->search.macro_count <= options.max_depth);
            CHECK(f2l::cross_solved(result->start));
            CHECK(f2l::f2l_solved(result->f2l_state));
            CHECK(f2l::normalise(result->f2l_state + last_layer) == identity);
            std::printf("Recovered F2L for %.*s (%.*s, %.*s; %d macros): %s\n",
                        static_cast<int>(fixture.name.size()),
                        fixture.name.data(),
                        static_cast<int>(fixture.single_time.size()),
                        fixture.single_time.data(),
                        static_cast<int>(fixture.recorded_at.size()),
                        fixture.recorded_at.data(), result->search.macro_count,
                        result->search.moves.c_str());
        }
    } catch (const std::exception& error) {
        std::printf("FAIL unknown-F2L search: %s\n", error.what());
        ++failures;
    }

    try {
        f2l::SearchOptions options;
        options.max_depth = 0;
        options.timeout = std::chrono::seconds{1};
        const auto result = f2l::reconstruct_unknown_f2l(
            "x2 R U x2", "U' R'", "", options);
        CHECK(result.has_value());
        if (result) {
            CHECK(result->inspection_index == 2);
            CHECK(result->inspection_moves == "x2");
            CHECK(result->compatible_frame_count > 1);
            CHECK(result->compatible_inspections.find("x2") !=
                  std::string::npos);
            CHECK(result->cross_face == "U");
            CHECK(result->search.moves.empty());
            CHECK(f2l::cross_solved(result->start));
            CHECK(result->f2l_state == identity);
        }
    } catch (const std::exception& error) {
        std::printf("FAIL generalized unknown-F2L search: %s\n",
                    error.what());
        ++failures;
    }

    std::printf("%s (%d failing checks)\n",
                failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
