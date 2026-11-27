// tests/test_core.cpp -- headless: must build WITHOUT SFML on the include path.
#include "cube_view.hpp"
#include "rubik.hpp"
#include "scramble.hpp"

#include <array>
#include <cstdio>
#include <random>
#include <string>

static int g_fail = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++g_fail; } } while (0)

static int order(const Move& m) {
    Move c = m;
    int n = 1;
    while (!(c == Move::identity())) {
        c = c + m;
        if (++n > 5000) return -1;
    }
    return n;
}

static int perm_parity(const uint8_t* p, int n) {
    int s = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (p[i] > p[j])
                s ^= 1;
    return s;
}

static bool throws(const char* s) {
    try {
        Rubik::parse(s);
    } catch (...) {
        return true;
    }
    return false;
}

static bool same(const char* a, const char* b) {
    try {
        return Rubik::parse(a) == Rubik::parse(b);
    } catch (...) {
        return false;
    }
}

int main() {
    const Move id = Move::identity();

    const Move* all[] = {
        &Rubik::U, &Rubik::R, &Rubik::F, &Rubik::D, &Rubik::L, &Rubik::B,
        &Rubik::M, &Rubik::E, &Rubik::S, &Rubik::x, &Rubik::y, &Rubik::z
    };
    for (const Move* m : all) {
        CHECK(order(*m) == 4);
        CHECK((*m + -*m) == id);
        CHECK((-*m + *m) == id);
    }

    CHECK(order("R U"_move) == 105);
    CHECK(order("R U R' U'"_move) == 6);
    CHECK(order("R U2 D' B D'"_move) == 1260);
    CHECK(("R U R' U R U2 R'"_move * 6) == id);

    std::mt19937 rng(1);
    const char* f[] = {"U", "R", "F", "D", "L", "B"};
    auto rnd = [&](int len) {
        std::string t;
        for (int i = 0; i < len; i++) {
            t += f[rng() % 6];
            int k = rng() % 3;
            if (k == 1) t += "'";
            if (k == 2) t += "2";
            t += " ";
        }
        return t;
    };
    for (int i = 0; i < 2000; i++) {
        Move a = Rubik::parse(rnd(20));
        Move b = Rubik::parse(rnd(20));
        Move c = Rubik::parse(rnd(20));
        CHECK(((a + b) + c) == (a + (b + c)));
        CHECK((a + -a) == id);
        int so = 0;
        for (int k = 0; k < 8; k++) so += a.co[k];
        CHECK(so % 3 == 0);
        int se = 0;
        for (int k = 0; k < 12; k++) se += a.eo[k];
        CHECK(se % 2 == 0);
        CHECK(perm_parity(a.cp.data(), 8) == perm_parity(a.ep.data(), 12));
    }

    CHECK((Rubik::y + Rubik::R + -Rubik::y) == Rubik::B);
    CHECK((Rubik::x + Rubik::U + -Rubik::x) == Rubik::F);
    CHECK((Rubik::z + Rubik::R + -Rubik::z) == Rubik::U);

    const f2l::Facelets solved_facelets = f2l::cube_facelets(id);
    for (std::size_t face = 0; face < 6; ++face) {
        std::array<int, 6> color_counts{};
        for (std::size_t position = 0; position < 9; ++position)
            ++color_counts[solved_facelets[face * 9 + position]];
        CHECK(solved_facelets[face * 9 + 4] == face);
        CHECK(color_counts[face] == 9);
    }
    const f2l::Facelets scrambled_facelets =
        f2l::cube_facelets(Rubik::parse("R U F2 L' D B"));
    std::array<int, 6> scrambled_color_counts{};
    for (std::uint8_t color : scrambled_facelets)
        ++scrambled_color_counts[color];
    for (int count : scrambled_color_counts) CHECK(count == 9);

    const std::string scramble_a = random_scramble(20, 42);
    CHECK(scramble_a == random_scramble(20, 42));
    CHECK(Rubik::parse(scramble_a) != id);
    CHECK(random_scramble(0, 42).empty());

    CHECK(same("F R'F' R", "F R' F' R"));
    CHECK(same("U2R", "U2 R"));
    CHECK(same("R2'", "R2"));
    CHECK(same("Rw", "r"));
    CHECK(same("  R   U  ", "R U"));

    CHECK(throws("Q"));
    CHECK(throws("R3"));
    CHECK(throws("R''"));
    CHECK(throws("'"));

    std::printf("%s (%d failing checks)\n", g_fail ? "FAILED" : "OK", g_fail);
    return g_fail ? 1 : 0;
}
