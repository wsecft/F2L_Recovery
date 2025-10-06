#pragma once

#include <string_view>
#include <cstring>
#include <SFML/Graphics.hpp>
#include <frozen/unordered_map.h>
#include "move.hpp"

struct Rubik {
    // base moves
    static constexpr Move U = Move{
        {3,0,1,2,4,5,6,7},
        {0,0,0,0,0,0,0,0},
        {1,2,3,0,4,5,6,7,8,9,10,11},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,2,3,4,5}
    };

    static constexpr Move R = Move{
        {4,1,2,0,7,5,6,3},
        {1,0,0,2,2,0,0,1},
        {0,8,2,3,4,9,6,7,5,1,10,11},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,2,3,4,5}
    };

    static constexpr Move F = Move{
        {1,5,2,3,0,4,6,7},
        {2,1,0,0,1,2,0,0},
        {11,1,2,3,8,5,6,7,0,9,10,4},
        {1,0,0,0,1,0,0,0,1,0,0,1},
        {0,1,2,3,4,5}
    };

    static constexpr Move D = Move{
        {0,1,2,3,5,6,7,4},
        {0,0,0,0,0,0,0,0},
        {0,1,2,3,7,4,5,6,8,9,10,11},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,2,3,4,5}
    };

    static constexpr Move L = Move{
        {0,2,6,3,4,1,5,7},
        {0,2,1,0,0,1,2,0},
        {0,1,2,10,4,5,6,11,8,9,7,3},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,2,3,4,5}
    };

    static constexpr Move B = Move{
        {0,1,3,7,4,5,2,6},
        {0,0,2,1,0,0,1,2},
        {0,1,9,3,4,5,10,7,8,6,2,11},
        {0,0,1,0,0,0,1,0,0,1,1,0},
        {0,1,2,3,4,5}
    };

    static constexpr Move M = Move{
            {0,1,2,3,4,5,6,7},
            {0,0,0,0,0,0,0,0},
            {2,1,6,3,0,5,4,7,8,9,10,11},
            {1,0,1,0,1,0,1,0,0,0,0,0},
            {5,1,0,2,4,3}
    };

    static constexpr Move E = Move{
            {0,1,2,3,4,5,6,7},
            {0,0,0,0,0,0,0,0},
            {0,1,2,3,4,5,6,7,11,8,9,10},
            {0,0,0,0,0,0,0,0,1,1,1,1},
            {0,2,4,3,5,1}
    };

    static constexpr Move S = Move{
            {0,1,2,3,4,5,6,7},
            {0,0,0,0,0,0,0,0},
            {0,3,2,7,4,1,6,5,8,9,10,11},
            {0,1,0,1,0,1,0,1,0,0,0,0},
            {4,0,2,1,3,5}
    };

    static constexpr Move x = -M + R + -L;
    static constexpr Move y = -E + U + -D;
    static constexpr Move z = S + F + -B;
    static constexpr Move u = -E + U;
    static constexpr Move r = -M + R;
    static constexpr Move f = S + F;
    static constexpr Move d = E + D;
    static constexpr Move l = M + L;
    static constexpr Move b = -S + B;


    // O(1) lookup, both at compile-time and runtime yay
    static constexpr auto move_map = frozen::make_unordered_map<char, const Move*, 18>({
        std::pair{'U', &U},
        std::pair{'R', &R},
        std::pair{'F', &F},
        std::pair{'D', &D},
        std::pair{'L', &L},
        std::pair{'B', &B},
        std::pair{'M', &M},
        std::pair{'S', &S},
        std::pair{'E', &E},
        std::pair{'x', &x},
        std::pair{'y', &y},
        std::pair{'z', &z},
        std::pair{'u', &u},
        std::pair{'r', &r},
        std::pair{'f', &f},
        std::pair{'d', &d},
        std::pair{'l', &l},
        std::pair{'b', &b}
     });


    static constexpr Move parse_move(std::string_view str) {
        if (str.size() == 2) {
            if (str[1] == '2') return *move_map.at(str[0]) * 2;
            if (str[1] == '\'') return -*move_map.at(str[0]);
            throw std::runtime_error{ "Wtf bro ? Sus tbh" };
        }
        return *move_map.at(str[0]);
    }

    static constexpr Move parse(std::string_view str) {
        Move result = Move::identity();

        while (!str.empty()) {
            // Trim spaces
            auto first_non_space = str.find_first_not_of(' ');
            if (first_non_space == std::string_view::npos) break;
            str.remove_prefix(first_non_space);

            // Find end of token
            auto space_pos = str.find(' ');
            std::string_view token = str.substr(0, space_pos);

            result = result + parse_move(token);

            // Advance past token
            if (space_pos == std::string_view::npos) break;
            str.remove_prefix(space_pos + 1);
        }

        return result;
    }
};

inline constexpr Move operator""_move(const char* str, std::size_t size)
{
    return Rubik::parse(std::string_view(str, size));
}