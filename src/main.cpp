#include "draw.hpp"
#include "solver.hpp"

#include <SFML/Graphics.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <stop_token>
#include <string>
#include <thread>

namespace {
std::mutex output_mutex;
std::mutex input_mutex;
std::queue<std::string> input_queue;
std::atomic<bool> input_eof{false};

void console_input_thread() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::lock_guard lock(input_mutex);
        input_queue.push(std::move(line));
    }
    input_eof.store(true);
}

void print_line(const std::string& line) {
    std::lock_guard lock(output_mutex);
    std::cout << line << '\n';
}
}

int main() {
    Move cube = Move::identity();
    std::jthread input_thread(console_input_thread);
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Rubik's Cube");
    window.setFramerateLimit(60);

    std::jthread solve_thread;
    std::atomic<bool> solver_running{false};

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

        std::string line;
        {
            std::lock_guard lock(input_mutex);
            if (!input_queue.empty()) {
                line = std::move(input_queue.front());
                input_queue.pop();
            }
        }

        if (!line.empty()) {
            if (line == "r") {
                cube = Move::identity();
            } else if (line == "quit") {
                window.close();
            } else if (line.rfind("scramble", 0) == 0) {
                int count = 20;
                std::istringstream args(line);
                std::string command;
                args >> command >> count;
                if (count < 0) {
                    print_line("error: scramble count must be non-negative");
                } else {
                    std::mt19937 rng(static_cast<unsigned>(std::chrono::steady_clock::now()
                        .time_since_epoch().count()));
                    const char* moves[] = {"U","U'","U2","R","R'","R2",
                                           "F","F'","F2","D","D'","D2",
                                           "L","L'","L2","B","B'","B2"};
                    for (int i = 0; i < count; ++i)
                        cube += Rubik::parse(moves[rng() % 18]);
                }
            } else if (line == "solve") {
                if (solver_running.exchange(true)) {
                    print_line("solve already running");
                } else {
                    Move start = cube;
                    solve_thread = std::jthread([start, &solver_running](std::stop_token stop) {
                        f2l::SearchOptions options;
                        options.max_depth = 4;
                        options.timeout = std::chrono::milliseconds{5000};
                        options.stop = stop;
                        options.use_pair_pruning = true;
                        auto result = f2l::search(start, f2l::goal_f2l(start), options);
                        if (result)
                            print_line("solution: " + result->moves);
                        else
                            print_line("no solution within depth " +
                                       std::to_string(options.max_depth));
                        solver_running.store(false);
                    });
                }
            } else if (line.rfind("recover ", 0) == 0) {
                std::size_t separator = line.find('|');
                if (separator == std::string::npos) {
                    print_line("error: recovery syntax: recover <start>|<end>");
                } else {
                    std::string start_text = line.substr(8, separator - 8);
                    std::string end_text = line.substr(separator + 1);
                    try {
                        Move start = Rubik::parse(start_text);
                        Move end = Rubik::parse(end_text);
                        if (solver_running.exchange(true)) {
                            print_line("solve already running");
                        } else {
                            solve_thread = std::jthread(
                                [start, end, &solver_running](std::stop_token stop) {
                                    f2l::SearchOptions options;
                                    options.max_depth = 4;
                                    options.timeout = std::chrono::milliseconds{5000};
                                    options.stop = stop;
                                    auto result = f2l::recover(start, end, options);
                                    if (result &&
                                        f2l::verify(start, result->moves,
                                                    f2l::goal_exact(end)))
                                        print_line("recovery: " + result->moves);
                                    else
                                        print_line("no solution within depth " +
                                                   std::to_string(options.max_depth));
                                    solver_running.store(false);
                                });
                        }
                    } catch (const std::invalid_argument& error) {
                        print_line(std::string("error: ") + error.what());
                    }
                }
            } else {
                try {
                    cube += Rubik::parse(line);
                } catch (const std::invalid_argument& error) {
                    print_line(std::string("error: ") + error.what());
                }
            }
        }

        {
            std::lock_guard lock(input_mutex);
            if (input_eof.load() && input_queue.empty())
                window.close();
        }

        window.clear(sf::Color::Black);
        draw_cube(window, cube, 30.f, {0.f, 0.f});
        window.display();
    }
    return 0;
}
