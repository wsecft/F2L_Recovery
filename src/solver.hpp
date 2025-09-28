#include "rubik.hpp"

class F2LPartialSolver {
public:
    Move findCandidateF2L(Move startState, Move endState) {
        startState = endState+ -startState;
    }
};