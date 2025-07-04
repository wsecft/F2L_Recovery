#include "rubik.hpp"
#include "draw.hpp"
#include <SFML/Graphics.hpp>
#include <optional>


// Apply a move to the cube using permutation and orientation deltas
void apply_move_old(Cube& cube, const Move& m) {
    uint8_t new_corners[8];
    uint8_t new_corner_orient[8];

    for (int i = 0; i < 8; i++) {
        uint8_t old_index = m.corner_perm[i];
        new_corners[i] = cube.corner_perm[old_index];
        new_corner_orient[i] = (cube.corner_orient[old_index] + m.corner_orient_delta[i]) % 3;
    }

    uint8_t new_edges[12];
    uint8_t new_edge_orient[12];

    for (int i = 0; i < 12; i++) {
        uint8_t old_index = m.edge_perm[i];
        new_edges[i] = cube.edge_perm[old_index];
        new_edge_orient[i] = cube.edge_orient[old_index] ^ m.edge_orient_delta[i];
    }

    std::memcpy(cube.corner_perm, new_corners, 8);
    std::memcpy(cube.corner_orient, new_corner_orient, 8);
    std::memcpy(cube.edge_perm, new_edges, 12);
    std::memcpy(cube.edge_orient, new_edge_orient, 12);
}

int main() {
    auto all_moves = generate_all_moves();
	Cube cube = Cube::identity();
    //apply_move_old(cube, all_moves["B"]);
    //apply_move_old(cube, all_moves["F'"]);
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Rubik's Cube Net");

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }

        window.clear(sf::Color::Black);
        draw_cube_net(window, cube, 30.f, {0.f, 0.f});
        window.display();
    }
    return 0;
}