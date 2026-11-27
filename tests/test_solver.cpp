#include "solver.hpp"

#include <cstdint>
#include <cstdio>
#include <random>
#include <string>

static int failures = 0;
#define CHECK(value) do { \
    if (!(value)) { \
        std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #value); \
        ++failures; \
    } \
} while (false)

int main() {
    const Move identity = Move::identity();

    CHECK(detail::triggers.size() == 16);
    CHECK(detail::transitions.size() == 7);
    CHECK(f2l::rotation_notations()[0].empty());
    CHECK(f2l::rotation_notations()[2] == "x2");
    for (std::size_t i = 1; i < f2l::rotations.size(); ++i) {
        CHECK(!f2l::rotation_notations()[i].empty());
        CHECK(Rubik::parse(f2l::rotation_notations()[i]) ==
              f2l::rotations[i]);
    }
    for (std::size_t i = 0; i < detail::combined.size(); ++i) {
        CHECK(detail::combined[i] + detail::invCombined[i] == identity);
    }

    CHECK(f2l::normalise(identity + Rubik::y) == identity);
    CHECK(f2l::pair_distance(identity, f2l::FR) == 0);
    CHECK(f2l::pair_distance(Rubik::R, f2l::FR) > 0);
    CHECK(f2l::pair_lower_bound(identity) == 0);
    for (const Move& rotation : f2l::rotations) {
        Move state = identity + Rubik::R + rotation;
        CHECK(f2l::normalise(state) == f2l::normalise(identity + Rubik::R));
    }

    CHECK(f2l::cross_solved(identity));
    CHECK(f2l::slot_solved(identity, f2l::FR));
    CHECK(f2l::slot_solved(identity, f2l::FL));
    CHECK(f2l::slot_solved(identity, f2l::BL));
    CHECK(f2l::slot_solved(identity, f2l::BR));
    CHECK(f2l::f2l_solved(identity));
    CHECK(f2l::goal_f2l(identity)(identity));
    CHECK(f2l::goal_slot(identity, f2l::FR)(identity));
    CHECK(!f2l::slot_solved(Rubik::R, f2l::FR));
    CHECK(!f2l::slot_solved(Rubik::F, f2l::FR));
    CHECK(!f2l::cross_solved(Rubik::R));

    Move start = "x2 U' L F' L2 R' B2"_move;
    Move end = start + detail::combined[27] + detail::combined[3];
    auto exact = f2l::search(start, f2l::goal_exact(end),
                             f2l::SearchOptions{2, std::chrono::milliseconds{5000}});
    CHECK(exact.has_value());
    if (exact) {
        CHECK(exact->macro_count <= 2);
        CHECK(exact->moves.find("U2R") == std::string::npos);
        CHECK(exact->moves.empty() || exact->moves.back() != ' ');
        CHECK(f2l::verify(start, exact->moves, f2l::goal_exact(end)));
    }

    std::mt19937 rng(123456);
    for (int k = 1; k <= 3; ++k) {
        Move chain = identity;
        for (int i = 0; i < k; ++i) {
            chain = chain + detail::combined[rng() % detail::combined.size()];
        }
        Move provable_start = -chain;
        auto result = f2l::search(provable_start, f2l::goal_exact(identity),
                                  f2l::SearchOptions{k, std::chrono::milliseconds{5000}});
        CHECK(result.has_value());
        if (result) {
            CHECK(result->macro_count <= k);
            CHECK(f2l::verify(provable_start, result->moves,
                              f2l::goal_exact(identity)));
        }
    }

    std::printf("%s (%d failing checks)\n",
                failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
