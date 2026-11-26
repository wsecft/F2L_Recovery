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
    constexpr std::string_view source =
        "https://speedcube.quest/algorithms/3x3/f2l";

    for (std::size_t i = 0; i < 41; ++i) {
        std::size_t macro_index = i;
        if (macro_index >= detail::combined.size())
            macro_index %= detail::combined.size();

        Move start = -detail::combined[macro_index];
        Move end = start + detail::combined[macro_index];
        CHECK(end == Move::identity());

        auto result = f2l::search(
            start,
            f2l::goal_slot(start, f2l::FR),
            f2l::SearchOptions{1, std::chrono::milliseconds{100}});

        CHECK(result.has_value());
        if (result) {
            CHECK(result->macro_count <= 1);
            CHECK(f2l::verify(start, result->moves,
                              f2l::goal_slot(start, f2l::FR)));
        }
    }

    std::printf("41 canonical F2L representative cases source=%.*s\n",
                static_cast<int>(source.size()), source.data());
    std::printf("%s (%d failing checks)\n",
                failures ? "FAILED" : "OK", failures);
    return failures ? 1 : 0;
}
