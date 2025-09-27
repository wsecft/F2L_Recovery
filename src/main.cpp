#include "rubik.hpp"
#include "draw.hpp"
#include <SFML/Graphics.hpp>
#include <optional>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstring>



std::atomic<bool> has_input{ false };
std::string input_buffer;

void console_input_thread() {
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        input_buffer = line;
        has_input = true;
    }
}


// Apply a move to the cube using permutation and orientation deltas
void apply_move_old(Cube& cube, const Move& m) {
    uint8_t new_corners[8];
    uint8_t new_corner_orient[8];

    for (int i = 0; i < 8; i++) {
        uint8_t old_index = m.corner_perm[i];
        new_corners[i] = cube.corner_perm[old_index];
        new_corner_orient[i] = (cube.corner_orient[old_index] + m.corner_orient[i]) % 3;
    }

    uint8_t new_edges[12];
    uint8_t new_edge_orient[12];

    for (int i = 0; i < 12; i++) {
        uint8_t old_index = m.edge_perm[i];
        new_edges[i] = cube.edge_perm[old_index];
        new_edge_orient[i] = cube.edge_orient[old_index] ^ m.edge_orient[i];
    }

    std::memcpy(cube.corner_perm, new_corners, 8);
    std::memcpy(cube.corner_orient, new_corner_orient, 8);
    std::memcpy(cube.edge_perm, new_edges, 12);
    std::memcpy(cube.edge_orient, new_edge_orient, 12);
}

int main() {
	Move cube = Move::identity();

    std::cout << "F'"_move;
    cube=cube+ "F'"_move;
    std::cout << cube;
    //std::cout << all_moves["F "];
    //std::cout << -"F"_move;
    //std::cout << std::boolalpha << ("U"_move == get_inverse("U'"_move)) << std::endl;
    //apply_move(cube, all_moves["U"]);
    //apply_move(cube, parse_move("U",all_moves));
    std::string temp;

    std::thread input_thread(console_input_thread);
    input_thread.detach(); // Don't wait for it on exit
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Rubik's Cube");

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

        if (has_input) {
            std::cout << "Received console input: " << input_buffer << "\n";
            has_input = false;
            if (input_buffer == "r") cube = Move::identity();
            cube = cube + Rubik::parse(input_buffer);
        }

        window.clear(sf::Color::Black);
        draw_cube_net(window, cube, 30.f, {0.f, 0.f});
        window.display();
    }
    return 0;
}
