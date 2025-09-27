#include <SFML/Graphics.hpp>
#include <array>
#include <map>
#include <string>
#include "rubik.hpp"

inline static const std::unordered_map<Cube::sides, sf::Color> face_colors = {
    {Cube::U, sf::Color::White},
    {Cube::D, sf::Color::Yellow},
    {Cube::F, sf::Color::Green},
    {Cube::B, sf::Color::Blue},
    {Cube::R, sf::Color::Red},
    {Cube::L, sf::Color(255, 165, 0)} // Orange
};

std::map<Cube::sides, std::array<sf::Color, 9>> cube_to_facelets(const Cube& cube) {
    std::map<Cube::sides, std::array<sf::Color, 9>> facelets;

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
			Cube::sides face = (Cube::sides)Cube::edge_facelets[i][j];
            int pos = edge_to_facelet[i][j];
            sf::Color color=face_colors.at((Cube::sides)Cube::edge_facelets[edge][ori^j]);
            facelets[(Cube::sides)face][pos] = color;
        }
    }
    for (int i = 0; i < 8; ++i) {
        int corner = cube.corner_perm[i];
        int ori = cube.corner_orient[i];

        for (int j = 0; j < 3; ++j) {
            Cube::sides face = (Cube::sides)Cube::corner_facelets[i][j];
            int pos = corner_to_facelet[i][j];
            sf::Color color = face_colors.at((Cube::sides)Cube::corner_facelets[corner][(ori+j)%3]);
            facelets[(Cube::sides)face][pos] = color;
        }
    }
    return facelets;
}


std::map<Cube::sides, std::array<sf::Color, 9>> cube_to_facelets_wrong(const Move& cube) {
    static const uint8_t edge_to_facelet[12][2][2] = {
        // UF (0)
        {{Cube::U, 7}, {Cube::F, 1}},
        // UR (1)
        {{Cube::U, 5}, {Cube::R, 1}},
        // UB (2)
        {{Cube::U, 1}, {Cube::B, 1}},
        // UL (3)
        {{Cube::U, 3}, {Cube::L, 1}},
        // DF (4)
        {{Cube::D, 1}, {Cube::F, 7}},
        // DR (5)
        {{Cube::D, 5}, {Cube::R, 7}},
        // DB (6)
        {{Cube::D, 7}, {Cube::B, 7}},
        // DL (7)
        {{Cube::D, 3}, {Cube::L, 7}},
        // FR (8)
        {{Cube::F, 5}, {Cube::R, 3}},
        // BR (9)
        {{Cube::B, 3}, {Cube::R, 5}},
        // BL (10)
        {{Cube::B, 5}, {Cube::L, 3}},
        // FL (11)
        {{Cube::F, 3}, {Cube::L, 5}},
    };

    static const int corner_to_facelet[8][3][2] = {
        // URF (0)
        {{Cube::U, 8}, {Cube::R, 0}, {Cube::F, 2}},
        // UFL (1)
        {{Cube::U, 6}, {Cube::F, 0}, {Cube::L, 2}},
        // ULB (2)
        {{Cube::U, 0}, {Cube::L, 0}, {Cube::B, 2}},
        // UBR (3)
        {{Cube::U, 2}, {Cube::B, 0}, {Cube::R, 2}},
        // DFR (4)
        {{Cube::D, 2}, {Cube::F, 8}, {Cube::R, 6}},
        // DLF (5)
        {{Cube::D, 0}, {Cube::L, 8}, {Cube::F, 6}},
        // DBL (6)
        {{Cube::D, 6}, {Cube::B, 8}, {Cube::L, 6}},
        // DRB (7)
        {{Cube::D, 8}, {Cube::R, 8}, {Cube::B, 6}}
    };

    std::map<Cube::sides, std::array<sf::Color, 9>> facelets;


    // Face center colors (they're fixed)
    for (auto [face, color] : face_colors)
        facelets[face][4] = color;

    for (int i = 0; i < 12; ++i) {
        int edge = cube.edge_perm[i];
        int ori = cube.edge_orient[i];

        for (int j = 0; j < 2; ++j) {
            auto [face, pos] = edge_to_facelet[edge][ori^j];
            sf::Color color = face_colors.at((Cube::sides)Cube::edge_facelets[i][j]); // or use cube logic
            facelets[(Cube::sides)face][pos] = color;
        }
    }
    for (int i = 0; i < 8; ++i) {
        int corner = cube.corner_perm[i];
        int ori = cube.corner_orient[i];

        for (int j = 0; j < 3; ++j) {
            auto [face, pos] = corner_to_facelet[corner][(ori+j)%3];
            sf::Color color = face_colors.at((Cube::sides)Cube::corner_facelets[i][j]);
            facelets[(Cube::sides)face][pos] = color;
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

void draw_cube_net(sf::RenderTarget& target, const Cube& cube, float size, sf::Vector2f origin) {
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
        draw_face(target, face_origin, size, facelets[(Cube::sides)f]);
    }
}