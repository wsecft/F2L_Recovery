#include "real_solve_fixtures.hpp"
#include "solver.hpp"

#include <cstdio>
#include <cstdint>
#include <string_view>

static int failures = 0;
#define CHECK(value) do { \
    if (!(value)) { \
        std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #value); \
        ++failures; \
    } \
} while (false)

int main() {
    for (const RealSolveFixture& fixture : real_solve_fixtures) {
        try {
            Move state = Rubik::parse(fixture.setup);
            std::printf("fixture %.*s %.*s\n",
                        static_cast<int>(fixture.id.size()), fixture.id.data(),
                        static_cast<int>(fixture.name.size()), fixture.name.data());

            for (const RealSolvePhase& phase : fixture.phases) {
                if (phase.moves.empty()) continue;
                Move segment = Rubik::parse(phase.moves);
                Move target = state + segment;
                CHECK(target == state + segment);
                CHECK(f2l::verify(state, phase.moves,
                                  f2l::goal_exact(target)));
                state = target;
            }
        } catch (const std::exception& error) {
            std::printf("FAIL fixture %.*s parse: %s\n",
                        static_cast<int>(fixture.id.size()), fixture.id.data(),
                        error.what());
            ++failures;
        }
    }

    const RealSolveFixture& nahm = real_solve_fixtures[0];
    Move start = Rubik::parse(nahm.setup);
    for (std::size_t i = 0; i < 3; ++i)
        start = start + Rubik::parse(nahm.phases[i].moves);

    Move pair_target = start + Rubik::parse(nahm.phases[3].moves);
    std::uint16_t before = f2l::solved_bits(start);
    std::uint16_t after = f2l::solved_bits(pair_target);
    CHECK(f2l::verify(start, nahm.phases[3].moves,
                      f2l::goal_exact(pair_target)));
    CHECK(((after ^ before) & after) != 0 ||
          before != after);

    std::printf("%s (%d failing checks)\n",
                failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
