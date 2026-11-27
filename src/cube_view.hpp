#pragma once

#include "move.hpp"

#include <array>
#include <cstdint>

namespace f2l {

using Facelets = std::array<std::uint8_t, 54>;

inline Facelets cube_facelets(const Move& cube) {
    constexpr int corner_faces[8][3] = {
        {0, 1, 2}, {0, 2, 4}, {0, 4, 5}, {0, 5, 1},
        {3, 2, 1}, {3, 4, 2}, {3, 5, 4}, {3, 1, 5}
    };
    constexpr int edge_faces[12][2] = {
        {0, 2}, {0, 1}, {0, 5}, {0, 4},
        {3, 2}, {3, 1}, {3, 5}, {3, 4},
        {2, 1}, {5, 1}, {5, 4}, {2, 4}
    };
    constexpr int edge_positions[12][2] = {
        {7, 1}, {5, 1}, {1, 1}, {3, 1},
        {1, 7}, {5, 7}, {7, 7}, {3, 7},
        {5, 3}, {3, 5}, {5, 3}, {3, 5}
    };
    constexpr int corner_positions[8][3] = {
        {8, 0, 2}, {6, 0, 2}, {0, 0, 2}, {2, 0, 2},
        {2, 8, 6}, {0, 8, 6}, {6, 8, 6}, {8, 8, 6}
    };

    Facelets result{};
    for (int position = 0; position < 12; ++position) {
        const int edge = cube.ep[position];
        const int orientation = cube.eo[position];
        for (int side = 0; side < 2; ++side) {
            const int face = edge_faces[position][side];
            const int offset = face * 9 + edge_positions[position][side];
            result[offset] = static_cast<std::uint8_t>(
                edge_faces[edge][orientation ^ side]);
        }
    }
    for (int position = 0; position < 8; ++position) {
        const int corner = cube.cp[position];
        const int orientation = cube.co[position];
        for (int side = 0; side < 3; ++side) {
            const int face = corner_faces[position][side];
            const int offset = face * 9 + corner_positions[position][side];
            result[offset] = static_cast<std::uint8_t>(
                corner_faces[corner][(orientation + side) % 3]);
        }
    }
    for (int face = 0; face < 6; ++face)
        result[face * 9 + 4] = cube.centers[face];
    return result;
}

} // namespace f2l
