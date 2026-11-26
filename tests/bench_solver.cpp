#include "solver.hpp"

#include <chrono>
#include <cstdio>
#include <random>

int main() {
    std::printf("case,depth,nodes,ms,result\n");

    Move main_start = "x2 U' L F' L2 R' B2"_move;
    Move main_end = -"F R' F' r U R U' r U' "_move;
    for (int depth = 1; depth <= 4; ++depth) {
        auto begin = std::chrono::steady_clock::now();
        auto result = f2l::search(main_start, f2l::goal_exact(main_end),
                                  f2l::SearchOptions{depth,
                                                     std::chrono::milliseconds{5000}});
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::printf("main,%d,%llu,%lld,%s\n", depth,
                    static_cast<unsigned long long>(result ? result->nodes : 0),
                    static_cast<long long>(ms),
                    result ? "found" : "none");
    }

    std::mt19937 rng(123456);
    for (int k = 1; k <= 3; ++k) {
        Move chain = Move::identity();
        for (int i = 0; i < k; ++i) {
            chain = chain + detail::combined[rng() % detail::combined.size()];
        }
        Move start = -chain;
        auto begin = std::chrono::steady_clock::now();
        auto result = f2l::search(start, f2l::goal_exact(Move::identity()),
                                  f2l::SearchOptions{k,
                                                     std::chrono::milliseconds{5000}});
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::printf("provable-%d,%d,%llu,%lld,%s\n", k, k,
                    static_cast<unsigned long long>(result ? result->nodes : 0),
                    static_cast<long long>(ms),
                    result ? "found" : "none");
    }
}
