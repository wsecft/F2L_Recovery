#pragma once
#include <iostream>
#include <cstdint>
#include <array>
struct Move {
    std::array<uint8_t, 8> cp{};
    std::array<uint8_t, 8> co{};
    std::array<uint8_t, 12> ep{};
    std::array<uint8_t, 12> eo{};
    std::array<uint8_t, 6> centers{};

    constexpr bool operator==(const Move&) const = default;
    constexpr Move& operator+=(const Move& m) {
        Move tmp{};

        for (int i = 0; i < 8; ++i) {
            tmp.cp[i] = cp[m.cp[i]];
            tmp.co[i] = (m.co[i] + co[m.cp[i]]) % 3;
        }

        for (int i = 0; i < 12; ++i) {
            tmp.ep[i] = ep[m.ep[i]];
            tmp.eo[i] = eo[m.ep[i]] ^ m.eo[i];
        }

        for (int i = 0; i < 6; ++i) {
            tmp.centers[i] = centers[m.centers[i]];
        }

        *this = tmp;
        return *this;
    }
    constexpr static Move identity() {
        return Move{
            {0,1,2,3,4,5,6,7},       // cp
            {0,0,0,0,0,0,0,0},       // co
            {0,1,2,3,4,5,6,7,8,9,10,11}, // ep
            {0,0,0,0,0,0,0,0,0,0,0,0},    // eo
            {0,1,2,3,4,5} // centers
        };
    }
};

constexpr Move operator+(Move a, const Move& b) {
    a += b;   // reuse +=
    return a; // returns by value
}

constexpr Move operator*(const Move& a, int num) {
    Move result = Move::identity();
    Move base = a;

    while (num > 0) {
        if (num & 1) {
            result = result + base;
        }
        base = base + base;
        num >>= 1;
    }
    return result;
}

/*constexpr Move operator-(const Move& m) {
    Move result = m;
    for (int i = 0; i < 8; i++) {
        result.cp[m.cp[i]] = i;
    }
    for (int i = 0; i < 12; i++) {
        result.ep[m.ep[i]] = i;
    }
    return result;
}*/

constexpr Move operator-(const Move& m) {
    Move result;

    // inverse corner permutation and corner orientation (mod 3)
    for (int i = 0; i < 8; ++i) {
        result.cp[m.cp[i]] = i;
        result.co[m.cp[i]] = (3 - m.co[i]) % 3; // negation mod 3
    }

    // inverse edge permutation and edge orientation (mod 2)
    for (int i = 0; i < 12; ++i) {
        result.ep[m.ep[i]] = i;
        result.eo[m.ep[i]] = m.eo[i] % 2; // same as (2 - m.eo[i]) % 2
    }

    for (int i = 0; i < 6; ++i) {
        result.centers[m.centers[i]] = i;
    }

    return result;
}

inline std::ostream& operator<<(std::ostream& s, Move m) {
    s << "cp: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.cp[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "co: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.co[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "ep: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.ep[i]) << (i < 11 ? ' ' : '\n');
    }

    s << "eo: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.eo[i]) << (i < 11 ? ' ' : '\n');
    }

    s << "centers";
    for (int i = 0; i < 6; i++) {
        s << int(m.centers[i]) << (i < 5 ? ' ' : '\n');
    }

    return s;
}