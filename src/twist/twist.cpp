#include <cstdint>
#include <ostream>
#include <string>
#include <insertionfinder/twist.hpp>
using InsertionFinder::Twist;


namespace {
    constexpr const char* twist_string[24] = {
        "", "U", "U2", "U'",
        "", "D", "D2", "D'",
        "", "R", "R2", "R'",
        "", "L", "L2", "L'",
        "", "F", "F2", "F'",
        "", "B", "B2", "B'"
    };
};


Twist::Twist(const char* string) {
    if (string[0] == '\0') {
        this->twist = 0;
    }
    uint_fast8_t offset;
    switch (string[0]) {
    case 'U':
        offset = 0;
        break;
    case 'D':
        offset = 4;
        break;
    case 'R':
        offset = 8;
        break;
    case 'L':
        offset = 12;
        break;
    case 'F':
        offset = 16;
        break;
    case 'B':
        offset = 20;
        break;
    default:
        this->twist = 0;
        return;
    }
    if (string[1] == '\0') {
        this->twist = offset + 1;
    } else if (string[2] != '\0') {
        this->twist = 0;
    } else if (string[1] == '2') {
        this->twist = offset + 2;
    } else if (string[1] == '\'') {
        this->twist = offset + 3;
    } else {
        this->twist = 0;
    }
}

std::ostream& operator<<(std::ostream& out, Twist twist) {
    return out << twist_string[twist.twist];
}

std::string Twist::str() const {
    return twist_string[this->twist];
}
