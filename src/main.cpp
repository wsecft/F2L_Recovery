#include "rubik.hpp"
#include "draw.hpp"
#include "solver.hpp"
#include <SFML/Graphics.hpp>
#include <optional>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstring>

#include <chrono>

Move test(std::string s,int n) {
    Move cube=Move::identity();
    auto start = std::chrono::system_clock::now();
    Move seq= Rubik::parse(s);
    for (int i = 0; i < n; i++)
        seq = -seq;
    auto end = std::chrono::system_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << elapsed.count() << " ms";
    return cube;
}

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
    //assert(F2LPartialSolver::findCandidateF2L("F R' F' r U R U' r' y2 R U R' D2 R'"_move+"R D2"_move, "r U R' U' r' F R F'"_move,2)=="R U' R' y2"_move);
	Move cube = Move::identity();

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
                auto start = std::chrono::system_clock::now();
                Move seq = Rubik::parse(input_buffer);
                cube += seq;
                int i = 1;
                while (cube != Move::identity()) {
                    cube += seq;
                    i++;
                }
                auto end = std::chrono::system_clock::now();
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                std::cout << elapsed.count() << " ns";
                std::cout << "Period : " << i << "\n";
            }
        }

        window.clear(sf::Color::Black);
        draw_cube(window, cube, 30.f, {0.f, 0.f});
        window.display();
    }
    return 0;
}
