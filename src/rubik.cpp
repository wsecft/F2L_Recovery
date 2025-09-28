#include "rubik.hpp"


std::ostream& operator<<(std::ostream& s, Move m) {
    s << "corner_perm: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.corner_perm[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "corner_orient_delta: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.corner_orient[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "edge_perm: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.edge_perm[i]) << (i < 11 ? ' ' : '\n');
    }

    s << "edge_orient_delta: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.edge_orient[i]) << (i < 11 ? ' ' : '\n');
    }

    return s;
}

