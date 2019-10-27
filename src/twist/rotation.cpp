#include <array>
#include <ostream>
#include <string>
#include <insertionfinder/twist.hpp>
using InsertionFinder::Rotation;


namespace {
    constexpr const char* rotation_string[24] = {
        "", "y", "y2", "y'",
        "x", "x y", "x y2", "x y'",
        "x2", "x2 y", "z2", "x2 y'",
        "x'", "x' y", "x' y2", "x' y'",
        "z", "z y", "z y2", "z y'",
        "z'", "z' y", "z' y2", "z' y'"
    };
};


const std::array<std::array<int, 24>, 24> Rotation::rotation_transform = generate_rotation_transform_table();

std::array<std::array<int, 24>, 24> Rotation::generate_rotation_transform_table() noexcept {
    static const auto& table = Rotation::rotation_permutation;
    std::array<std::array<int, 24>, 24> center_transform;
    for (size_t i = 0; i < 24; ++i) {
        for (size_t j = 0; j < 24; ++j) {
            int x = table[i][0];
            Rotation center0 = table[j][x >> 1] ^ (x & 1);
            int y = table[i][1];
            Rotation center1 = table[j][y >> 1] ^ (y & 1);
            for (int k = 0; k < 24; ++k) {
                if (center0 == table[k][0] && center1 == table[k][1]) {
                    center_transform[i][j] = k;
                    break;
                }
            }
        }
    }
    return center_transform;
}


std::ostream& operator<<(std::ostream& out, Rotation rotation) {
    return out << rotation_string[rotation.rotation];
}

std::string Rotation::str() const {
    return rotation_string[this->rotation];
}
