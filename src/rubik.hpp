#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
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


    static constexpr bool is_space(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static constexpr bool is_face(char face) {
        switch (face) {
            case 'U':
            case 'R':
            case 'F':
            case 'D':
            case 'L':
            case 'B':
            case 'M':
            case 'E':
            case 'S':
            case 'x':
            case 'y':
            case 'z':
            case 'u':
            case 'r':
            case 'f':
            case 'd':
            case 'l':
            case 'b':
                return true;
            default:
                return false;
        }
    }

    static constexpr const Move* face_move(char face) {
        switch (face) {
            case 'U': return &U;
            case 'R': return &R;
            case 'F': return &F;
            case 'D': return &D;
            case 'L': return &L;
            case 'B': return &B;
            case 'M': return &M;
            case 'E': return &E;
            case 'S': return &S;
            case 'x': return &x;
            case 'y': return &y;
            case 'z': return &z;
            case 'u': return &u;
            case 'r': return &r;
            case 'f': return &f;
            case 'd': return &d;
            case 'l': return &l;
            case 'b': return &b;
            default: return nullptr;
        }
    }

    static constexpr Move parse_move(std::string_view str) {
        std::size_t i = 0;
        while (i < str.size() && is_space(str[i])) ++i;
        if (i == str.size()) throw std::invalid_argument{"empty move"};

        const char face = str[i++];
        const Move* move = face_move(face);
        if (move == nullptr) {
            if (face == 'w') throw std::invalid_argument{"wide move requires a face"};
            throw std::invalid_argument{"invalid move face"};
        }

        if (i < str.size() && str[i] == 'w') {
            if (face < 'A' || face > 'Z') throw std::invalid_argument{"invalid wide move"};
            ++i;
            char wide_face = '\0';
            switch (face) {
                case 'U': wide_face = 'u'; break;
                case 'R': wide_face = 'r'; break;
                case 'F': wide_face = 'f'; break;
                case 'D': wide_face = 'd'; break;
                case 'L': wide_face = 'l'; break;
                case 'B': wide_face = 'b'; break;
            }
            move = face_move(wide_face);
        }

        while (i < str.size() && is_space(str[i])) ++i;
        if (i == str.size()) return *move;

        if (i + 1 < str.size() && str[i] == '2' && str[i + 1] == '\'') i += 2;
        else if (i + 1 < str.size() && str[i] == '\'' && str[i + 1] == '2') i += 2;
        else if (str[i] == '2') ++i;
        else if (str[i] == '\'') ++i;
        else throw std::invalid_argument{"invalid move modifier"};

        while (i < str.size() && is_space(str[i])) ++i;
        if (i != str.size()) throw std::invalid_argument{"unexpected move suffix"};

        if (str.find("2") != std::string_view::npos)
            return *move * 2;
        if (str.find('\'') != std::string_view::npos)
            return -*move;
        return *move;
    }

    static constexpr Move parse(std::string_view str) {
        Move result = Move::identity();
        std::size_t i = 0;

        while (i < str.size()) {
            while (i < str.size() && is_space(str[i])) ++i;
            if (i == str.size()) break;

            const char face = str[i];
            if (!is_face(face)) {
                throw std::invalid_argument{"invalid move face"};
            }

            ++i;
            const Move* move = face_move(face);

            if (i < str.size() && str[i] == 'w') {
                if (face < 'A' || face > 'Z') throw std::invalid_argument{"invalid wide move"};
                ++i;
                switch (face) {
                    case 'U': move = &u; break;
                    case 'R': move = &r; break;
                    case 'F': move = &f; break;
                    case 'D': move = &d; break;
                    case 'L': move = &l; break;
                    case 'B': move = &b; break;
                }
            }

            if (i < str.size() && str[i] == '2') {
                ++i;
                if (i < str.size() && str[i] == '\'') ++i;
                result = result + *move * 2;
            } else if (i < str.size() && str[i] == '\'') {
                ++i;
                if (i < str.size() && str[i] == '2') ++i;
                result = result + -*move;
            } else {
                result = result + *move;
            }
        }

        return result;
    }
};

inline constexpr Move operator""_move(const char* str, std::size_t size)
{
    return Rubik::parse(std::string_view(str, size));
}