#include "rubik.hpp"


std::ostream& operator<<(std::ostream& s, Move m) {
    s << "cp: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.cp[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "co: ";
    for (int i = 0; i < 8; i++) {
        s << int(m.co[i]) << (i < 7 ? ' ' : '\n');
    }

    s << "ep: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.ep[i]) << (i < 11 ? ' ' : '\n');
    }

    s << "eo: ";
    for (int i = 0; i < 12; i++) {
        s << int(m.eo[i]) << (i < 11 ? ' ' : '\n');
    }

    return s;
}

