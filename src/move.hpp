struct Move {
    std::array<uint8_t, 8> corner_perm{};
    std::array<uint8_t, 8> corner_orient{};
    std::array<uint8_t, 12> edge_perm{};
    std::array<uint8_t, 12> edge_orient{};
    std::array<uint8_t, 6> center_perm{};

    constexpr bool operator==(const Move&) const = default;
    constexpr static Move identity() {
        return Move{
            {0,1,2,3,4,5,6,7},       // corner_perm
            {0,0,0,0,0,0,0,0},       // corner_orient
            {0,1,2,3,4,5,6,7,8,9,10,11}, // edge_perm
            {0,0,0,0,0,0,0,0,0,0,0,0}    // edge_orient
        };
    }
};

std::ostream& operator<<(std::ostream&, Move m);
constexpr Move operator+(const Move& a, const Move& b) {
    Move result{};

    for (int i = 0; i < 8; ++i) {
        result.corner_perm[i] = a.corner_perm[b.corner_perm[i]];
        result.corner_orient[i] =
            (b.corner_orient[i] + a.corner_orient[b.corner_perm[i]]) % 3;
    }

    for (int i = 0; i < 12; ++i) {
        result.edge_perm[i] = a.edge_perm[b.edge_perm[i]];
        result.edge_orient[i] =
            b.edge_orient[i] ^ a.edge_orient[b.edge_perm[i]];
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
        result.corner_perm[m.corner_perm[i]] = i;
    }
    for (int i = 0; i < 12; i++) {
        result.edge_perm[m.edge_perm[i]] = i;
    }
    return result;
}