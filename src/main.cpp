#include "draw.hpp"
#include "solver.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_set>

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
    std::cout << line << '\n' << std::flush;
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
            } else if (line.rfind("unknown", 0) == 0) {
                std::istringstream args(line);
                std::string command;
                std::string extra;
                std::chrono::seconds::rep seconds = 0;
                const auto max_seconds = std::chrono::duration_cast<
                    std::chrono::seconds>(
                        std::chrono::steady_clock::time_point::max() -
                        std::chrono::steady_clock::now()).count();
                if (!(args >> command >> seconds) || command != "unknown" ||
                    seconds <= 0 || seconds > max_seconds || (args >> extra)) {
                    print_line("error: syntax: unknown <positive-seconds>");
                } else if (solver_running.exchange(true)) {
                    print_line("solve already running");
                } else {
                    solve_thread = std::jthread(
                        [seconds, &solver_running](std::stop_token stop) {
                            try {
                                const Move inspection = Rubik::parse("x2");
                                const Move scramble = Rubik::parse(
                                    "L2 U2 L D' F R F' R2 D2 L' B2 R' "
                                    "D2 R2 U2 L B2 R D B");
                                const Move cross = Rubik::parse("U' L F' L2 R' B2");
                                const Move last_layer = Rubik::parse(
                                    "F R' F' r U R U' r' U'");

                                // Express the known cross in the x2 inspection frame.
                                const Move start =
                                    inspection + scramble + inspection + cross;
                                if (!f2l::cross_solved(start)) {
                                    print_line("error: built-in cross is not solved");
                                } else {
                                    const Move target = -last_layer;
                                    const Move identity = Move::identity();
                                    const f2l::Goal completes_solve =
                                        [last_layer, identity](const Move& state) {
                                            return f2l::f2l_solved(state) &&
                                                f2l::normalise(state + last_layer) ==
                                                    identity;
                                        };

                                constexpr int max_macro_depth = 16;
                                constexpr std::size_t min_beam_width = 500;
                                constexpr std::size_t max_beam_width = 10000;
                                constexpr std::size_t beam_step = 500;
                                constexpr auto search_slice =
                                    std::chrono::seconds{5};
                                // Cycle beam widths at the cap until the deadline.
                                const auto deadline =
                                    std::chrono::steady_clock::now() +
                                    std::chrono::seconds{seconds};
                                int best_macro_count = max_macro_depth + 1;
                                std::unordered_set<std::string> reported;
                                std::size_t beam_width = min_beam_width;
                                bool widening = true;

                                while (!stop.stop_requested()) {
                                    const auto remaining =
                                        std::chrono::duration_cast<
                                            std::chrono::milliseconds>(
                                            deadline -
                                            std::chrono::steady_clock::now());
                                    if (remaining <=
                                        std::chrono::milliseconds::zero())
                                        break;

                                    f2l::SearchOptions options;
                                    options.max_depth = std::min(
                                        best_macro_count, max_macro_depth);
                                    options.timeout = std::min(
                                        remaining,
                                        std::chrono::duration_cast<
                                            std::chrono::milliseconds>(
                                            search_slice));
                                    options.stop = stop;
                                    const auto result =
                                        f2l::search_f2l_target(
                                            start, target, options,
                                            beam_width);
                                    if (result) {
                                        if (!f2l::verify(
                                                start, result->moves,
                                                completes_solve)) {
                                            print_line(
                                                "error: unknown search "
                                                "returned an invalid F2L");
                                            break;
                                        }
                                        if (result->macro_count <=
                                                best_macro_count &&
                                            reported.insert(result->moves)
                                                .second) {
                                            best_macro_count = std::min(
                                                best_macro_count,
                                                result->macro_count);
                                            print_line(
                                                "unknown F2L (" +
                                                std::to_string(
                                                    result->macro_count) +
                                                " macros): " +
                                                result->moves);
                                        }
                                    }

                                    if (widening) {
                                        if (beam_width >= max_beam_width) {
                                            widening = false;
                                            beam_width -= beam_step;
                                        } else {
                                            beam_width = std::min(
                                                beam_width + beam_step,
                                                max_beam_width);
                                        }
                                    } else if (beam_width <=
                                               min_beam_width) {
                                        widening = true;
                                        beam_width += beam_step;
                                    } else {
                                        beam_width -= beam_step;
                                    }
                                }

                                if (best_macro_count >
                                    max_macro_depth) {
                                    print_line(
                                        stop.stop_requested()
                                            ? "unknown search cancelled; "
                                              "no F2L found"
                                            : "unknown: no F2L found within "
                                              "depth 16 and " +
                                                  std::to_string(seconds) +
                                                  " seconds");
                                } else {
                                    print_line(
                                        std::string(
                                            stop.stop_requested()
                                                ? "unknown search cancelled; "
                                                  "shortest found: "
                                                : "unknown search finished; "
                                                  "shortest found: ") +
                                        std::to_string(best_macro_count) +
                                        " macros");
                                    }
                                }
                            } catch (const std::exception& error) {
                                print_line(
                                    std::string("unknown search failed: ") +
                                    error.what());
                            }
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
