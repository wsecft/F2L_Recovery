#include "canonical_f2l_fixtures.hpp"
#include "solver.hpp"

#include <cstdio>
#include <cstdint>

static int failures = 0;
#define CHECK(value) do { \
    if (!(value)) { \
        std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #value); \
        ++failures; \
    } \
} while (false)

int main() {
    CHECK(canonical_f2l_fixtures.size() == 41);

    for (const CanonicalF2LFixture& fixture : canonical_f2l_fixtures) {
        Move solution = Rubik::parse(fixture.solution);
        Move start = -solution;

        CHECK(start + solution == Move::identity());
        CHECK(f2l::verify(start, fixture.solution,
                          f2l::goal_exact(Move::identity())));

        Move solved_after_solution = start + solution;
        CHECK(f2l::slot_solved(solved_after_solution, f2l::FR));
        CHECK(f2l::f2l_solved(solved_after_solution));

        std::printf("canonical %.*s group=%.*s\n",
                    static_cast<int>(fixture.id.size()), fixture.id.data(),
                    static_cast<int>(fixture.group.size()), fixture.group.data());
    }

    std::printf("41 canonical F2L cases source=%.*s\n",
                static_cast<int>(canonical_f2l_source.size()),
                canonical_f2l_source.data());
    std::printf("%s (%d failing checks)\n",
                failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
