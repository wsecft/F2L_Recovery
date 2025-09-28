struct Move {
    std::array<uint8_t, 8> cp{};
    std::array<uint8_t, 8> co{};
    std::array<uint8_t, 12> ep{};
    std::array<uint8_t, 12> eo{};
    std::array<uint8_t, 6> center_perm{};

    constexpr bool operator==(const Move&) const = default;
    constexpr static Move identity() {
        return Move{
            {0,1,2,3,4,5,6,7},       // cp
            {0,0,0,0,0,0,0,0},       // co
            {0,1,2,3,4,5,6,7,8,9,10,11}, // ep
            {0,0,0,0,0,0,0,0,0,0,0,0}    // eo
        };
    }
};

std::ostream& operator<<(std::ostream&, Move m);
constexpr Move operator+(const Move& a, const Move& b) {
    Move result{};

    for (int i = 0; i < 8; ++i) {
        result.cp[i] = a.cp[b.cp[i]];
        result.co[i] =
            (b.co[i] + a.co[b.cp[i]]) % 3;
    }

    for (int i = 0; i < 12; ++i) {
        result.ep[i] = a.ep[b.ep[i]];
        result.eo[i] =
            b.eo[i] ^ a.eo[b.ep[i]];
    }

    return result;
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

constexpr Move operator-(const Move& m) {
    Move result = m;
    for (int i = 0; i < 8; i++) {
        result.cp[m.cp[i]] = i;
    }
    for (int i = 0; i < 12; i++) {
        result.ep[m.ep[i]] = i;
    }
    return result;
}