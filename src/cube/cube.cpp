#include <cstdint>
#include <cstring>
#include <array>
#include <istream>
#include <functional>
#include <memory>
#include <ostream>
#include <vector>
#include <insertionfinder/cube.hpp>
using std::size_t;
using std::uint32_t;
using InsertionFinder::Cube;
using InsertionFinder::CubeStreamError;


void Cube::save_to(std::ostream& out) const {
    char data[21];
    for (size_t i = 0; i < 8; ++i) {
        data[i] = this->corner[i];
    }
    for (size_t i = 0; i < 12; ++i) {
        data[i + 8] = this->edge[i];
    }
    data[20] = this->_placement;
    out.write(data, 21);
}

void Cube::read_from(std::istream& in) {
    char data[21];
    in.read(data, 21);
    if (in.gcount() != 21) {
        throw CubeStreamError();
    }
    for (size_t i = 0; i < 8; ++i) {
        this->corner[i] = data[i];
    }
    for (size_t i = 0; i < 12; ++i) {
        this->edge[i] = data[i + 8];
    }
    this->_placement = data[20];
}


void Cube::inverse() noexcept {
    unsigned corner[8];
    unsigned edge[12];
    for (unsigned i = 0; i < 8; ++i) {
        unsigned item = this->corner[i];
        corner[item / 3] = i * 3 + (24 - item) % 3;
    }
    std::memcpy(this->corner, corner, 8 * sizeof(unsigned));
    for (unsigned i = 0; i < 12; ++i) {
        unsigned item = this->edge[i];
        edge[item >> 1] = i << 1 | (item & 1);
    }
    std::memcpy(this->edge, edge, 12 * sizeof(unsigned));
    this->_placement = Cube::inverse_center[this->_placement];
}

Cube Cube::inverse(const Cube& cube) noexcept {
    Cube result(Cube::raw_construct);
    for (unsigned i = 0; i < 8; ++i) {
        unsigned item = cube.corner[i];
        result.corner[item / 3] = i * 3 + (24 - item) % 3;
    }
    for (unsigned i = 0; i < 12; ++i) {
        unsigned item = cube.edge[i];
        result.edge[item >> 1] = i << 1 | (item & 1);
    }
    result._placement = Cube::inverse_center[cube._placement];
    return result;
}


uint32_t Cube::mask() const noexcept {
    static constexpr uint32_t center_mask[4] = {0, 3, 12, 15};
    uint32_t mask = 0;
    for (unsigned i = 0; i < 8; ++i) {
        if (this->corner[i] != i * 3) {
            mask |= 1 << i;
        }
    }
    for (unsigned i = 0; i < 12; ++i) {
        if (this->edge[i] != i << 1) {
            mask |= 1 << (i + 8);
        }
    }
    mask |= center_mask[Cube::center_cycles[this->_placement]] << 20;
    return mask;
}


size_t std::hash<Cube>::operator()(const Cube& cube) const noexcept {
    size_t result = 0;
    for (size_t i = 0; i < 8; ++i) {
        result = result * 31 + cube.corner[i];
    }
    for (size_t i = 0; i < 12; ++i) {
        result = result * 31 + cube.edge[i];
    }
    result = result * 31 + cube._placement;
    return result;
}
