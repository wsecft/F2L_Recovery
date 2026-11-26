#include "solver.hpp"

#include <cstddef>
#include <iostream>

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "check failed at " << __FILE__ << ':' << __LINE__ \
                      << ": " #condition << '\n'; \
            return 1; \
        } \
    } while (false)

int main() {
    CHECK(Rubik::parse("R R'") == Move::identity());
    CHECK(Rubik::parse("R R R R") == Move::identity());
    for (std::size_t i = 0; i < F2LPartialSolver::combined.size(); ++i) {
        CHECK(F2LPartialSolver::combined[i] + F2LPartialSolver::invCombined[i] == Move::identity());
    }
}
