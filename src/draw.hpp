#include <SFML/Graphics.hpp>
#include <array>
#include <map>
#include <string>
#include "rubik.hpp"


enum sides {
    U, R, F, D, L, B
};

inline static const int corner_facelets[8][3] = {
    {U, R, F}, {U, F, L}, {U, L, B}, {U, B, R},
    {D, F, R}, {D, L, F}, {D, B, L}, {D, R, B}
};

inline static const int edge_facelets[12][2] = {
    {U, F}, {U, R}, {U, B}, {U, L},
    {D, F}, {D, R}, {D, B}, {D, L},
    {F, R}, {B, R}, {B, L}, {F, L}
};


inline static const std::unordered_map<sides, sf::Color> face_colors = {
    {U, sf::Color::White},
    {D, sf::Color::Yellow},
    {F, sf::Color::Green},
    {B, sf::Color::Blue},
    {R, sf::Color::Red},
    {L, sf::Color(255, 165, 0)} // Orange
};

std::map<sides, std::array<sf::Color, 9>> cube_to_facelets(const Move& cube) {
    std::map<sides, std::array<sf::Color, 9>> facelets;

    static const uint8_t edge_to_facelet[12][2] = {
        {7,1},{5,1},{1,1},{3,1},//U edges
        {1,7},{5,7},{7,7},{3,7},//D edges
		{5,3},{3,5},{5,3},{3,5} //E slice edges
    };
    static const int corner_to_facelet[8][3] = {
        {8,0,2}, {6, 0, 2}, {0, 0, 2}, {2, 0, 2}, // U corners
		{2, 8, 6}, {0, 8, 6}, {6, 8, 6}, {8, 8, 6} // D corners
    };
    // Face center colors (they're fixed)
    for (auto [face, color] : face_colors)
        facelets[face][4] = color;

    for (int i = 0; i < 12; ++i) {
        int edge = cube.edge_perm[i];
        int ori = cube.edge_orient[i];

        for (int j = 0; j < 2; ++j) {
			sides face = (sides)edge_facelets[i][j];
            int pos = edge_to_facelet[i][j];
            sf::Color color=face_colors.at((sides)edge_facelets[edge][ori^j]);
            facelets[(sides)face][pos] = color;
        }
    }
    for (int i = 0; i < 8; ++i) {
        int corner = cube.corner_perm[i];
        int ori = cube.corner_orient[i];

        for (int j = 0; j < 3; ++j) {
            sides face = (sides)corner_facelets[i][j];
            int pos = corner_to_facelet[i][j];
            sf::Color color = face_colors.at((sides)corner_facelets[corner][(ori+j)%3]);
            facelets[(sides)face][pos] = color;
        }
    }
    return facelets;
}

void draw_face(sf::RenderTarget& target, sf::Vector2f origin, float size, const std::array<sf::Color, 9>& colors) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            sf::RectangleShape square({ size - 1.f, size - 1.f });
            square.setPosition({ origin.x + col * size, origin.y + row * size });
            square.setFillColor(colors[row * 3 + col]);
            square.setOutlineThickness(1.f);
            square.setOutlineColor(sf::Color::Black);
            target.draw(square);
        }
    }
}

void draw_cube_net(sf::RenderTarget& target, const Move& cube, float size, sf::Vector2f origin) {
    auto facelets = cube_to_facelets(cube);

    // Net layout (3x4 grid):
    //     [ ][U][ ][ ]
    //     [L][F][R][B]
    //     [ ][D][ ][ ]

    sf::Vector2f face_offsets[6] = {
        {size * 3,       0.f       }, // U
        {size * 6,       size * 3  }, // R
        {size * 3,       size * 3  }, // F
        {size * 3,       size * 6  }, // D
        {0.f,            size * 3  }, // L
        {size * 9,       size * 3  }  // B
    };

    for (int f = 0; f < 6; ++f) {
        sf::Vector2f face_origin = origin + face_offsets[f];
        draw_face(target, face_origin, size, facelets[(sides)f]);
    }
}