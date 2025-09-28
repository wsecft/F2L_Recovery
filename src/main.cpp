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

int main() {
	Move cube = Move::identity();
    //cube += "S"_move;
    static_assert("R U R' U'"_move * 5 == -"R U R' U'"_move);

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
            else {
                Move seq = Rubik::parse(input_buffer);
                cube += seq;
                int i = 1;
                while (cube != Move::identity()) {
                    cube += seq;
                    i++;
                }
                std::cout << "Period : " << i << "\n";
            }
        }

        window.clear(sf::Color::Black);
        draw_cube(window, cube, 30.f, {0.f, 0.f});
        window.display();
    }
    return 0;
}
