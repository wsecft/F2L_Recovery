#include "move.hpp"
#include <cmath>
#include <vector>
#include <cstdint>
class F2LPartialSolver {
    inline static constexpr std::array<std::string_view, 8> triggers = {
        "R U R'", "R U' R'", "R U2 R'", "R' F R F'",
        "R' U R", "R' U' R", "R' U2 R",  "F R'F' R"
    };

    inline static constexpr std::array<std::string_view, 5> transitions = {
        "U", "U'", "U2", "", "y"
    };

    template<size_t T, size_t U>
    static consteval auto makeCombined() {
        std::array<Move, T* U> arr{};
        for (size_t ti = 0; ti < T; ++ti) {
            for (size_t ui = 0; ui < U; ++ui) {
                size_t k = ti * U + ui;

                Move tmp = Rubik::parse(triggers[ti]);
                tmp += Rubik::parse(transitions[ui]);   // trigger + transition

                arr[k] = tmp;
            }
        }
        return arr;
    }

    template<size_t T, size_t U>
    static consteval auto makeInvCombined() {
        std::array<Move, T* U> arr{};
        for (size_t ti = 0; ti < T; ++ti) {
            for (size_t ui = 0; ui < U; ++ui) {
                size_t k = ti * U + ui;

                Move tmp = -Rubik::parse(transitions[ui]); // inverse transition
                tmp += -Rubik::parse(triggers[ti]);        // inverse trigger

                arr[k] = tmp;
            }
        }
        return arr;
    }

public:

    inline static constexpr auto combined =
        makeCombined<triggers.size(), transitions.size()>();

    inline static constexpr auto invCombined =
        makeInvCombined<triggers.size(), transitions.size()>();


    static std::vector<int>
        findCandidateF2L_(const Move& startState, const Move& endState, int depth)
    {
        using size_t = std::size_t;

        const size_t T = triggers.size();
        const size_t U = transitions.size();
        const size_t B = T * U; // branching factor

        auto apply = [&](int idx, Move& s) { s += combined[idx]; };
        auto undo = [&](int idx, Move& s) { s += invCombined[idx]; };

        Move tmp = startState;

        std::vector<int> path;        // current path
        std::vector<int> nextTry;     // next index to try at each depth level
        std::vector<int> best;        // BEST (shortest) solution found so far

        nextTry.push_back(0);

        while (!nextTry.empty()) {

            int idx = nextTry.back();

            // exceeded branching on this level -> backtrack
            if (idx >= (int)B) {
                nextTry.pop_back();
                if (!path.empty()) {
                    int last = path.back();
                    undo(last, tmp);
                    path.pop_back();
                }
                if (!nextTry.empty())
                    nextTry.back()++;
                continue;
            }

            // branch bounding: prune longer than current best solution
            if (!best.empty() && path.size() >= best.size())
            {
                // skip this candidate entirely
                nextTry.back()++;
                continue;
            }

            // apply this move
            path.push_back(idx);
            apply(idx, tmp);

            // goal check
            if (tmp == endState) {
                // if better (or first), store
                if (best.empty() || path.size() < best.size())
                    best = path;

                // backtrack immediately (don't go deeper)
                int last = path.back();
                undo(last, tmp);
                path.pop_back();
                nextTry.back()++;
                continue;
            }

            // full depth reached -> backtrack
            if ((int)path.size() == depth) {
                int last = path.back();
                undo(last, tmp);
                path.pop_back();
                nextTry.back()++;
                continue;
            }

            // descend
            nextTry.push_back(0);
        }

        return best; // empty if none found
    }

    static std::string findCandidateF2L(const Move& startState, const Move& endState, int depth) {
        auto v = findCandidateF2L_(startState, endState, depth);
        std::string ret;
        for (int k : v) {
            size_t i = k / transitions.size(); // trigger index
            size_t j = k % transitions.size();
            ret += std::string(triggers[i]) + " " + std::string(transitions[j]);
            if (transitions[i] != "") ret += " ";
        }
        assert(Rubik::parse(ret) == endState +- startState);
        return ret;
    }

};

/* GRAPH EXPLORATION INSTRUCTIONS :
* If on last branch, pop
* If not, push
* 
* 
* 
*/