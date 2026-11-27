#include <SFML/Graphics.hpp>
#include <array>
#include <unordered_map>
#include "cube_view.hpp"
#include "move.hpp"


enum sides {
    U, R, F, D, L, B
};

inline static const std::unordered_map<sides, sf::Color> face_colors = {
    {U, sf::Color::White},
    {D, sf::Color::Yellow},
    {F, sf::Color::Green},
    {B, sf::Color::Blue},
    {R, sf::Color::Red},
    {L, sf::Color(255, 110, 0)} // Orange
};

std::array<std::array<sf::Color, 9>, 6> cube_to_facelets(const Move& cube) {
    std::array<std::array<sf::Color, 9>, 6> facelets{};
    const f2l::Facelets stickers = f2l::cube_facelets(cube);
    for (int i = 0; i < 6; ++i) {
        for (int position = 0; position < 9; ++position)
            facelets[i][position] =
                face_colors.at(static_cast<sides>(stickers[i * 9 + position]));
    }
    return facelets;
}

void draw_face(sf::RenderTarget& target, sf::Vector2f origin, float size, const std::array<sf::Color, 9>& colors) {
    sf::RectangleShape square({size - 1.f, size - 1.f});
    square.setOutlineThickness(1.f);
    square.setOutlineColor(sf::Color::Black);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            square.setPosition({ origin.x + col * size, origin.y + row * size });
            square.setFillColor(colors[row * 3 + col]);
            target.draw(square);
        }
    }
}

void draw_cube(sf::RenderTarget& target, const Move& cube, float size, sf::Vector2f origin) {
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