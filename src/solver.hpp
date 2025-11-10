#include "move.hpp"

class F2LPartialSolver {
    inline static constexpr std::array<Move,8> triggers=
    {"R U R'"_move, "R U' R'"_move, "R U2 R'"_move, "R' F R F'"_move, "R U R'"_move, "R' U' R"_move, "R' U2 R"_move, "F R'F' R"_move};
public:
    static Move findCandidateF2L(Move startState, Move endState, int depth) {
        Move tmp=startState;
        /*while (tmp != endState) {
            
        }*/
        return tmp;
    }
};