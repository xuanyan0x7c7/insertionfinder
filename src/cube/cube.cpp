#include <cstdint>
#include <istream>
#include <functional>
#include <memory>
#include <ostream>
#include <fallbacks/optional.hpp>
#include <cube.hpp>
using namespace std;
using namespace InsertionFinder;


Cube::Cube() noexcept {
    for (int i = 0; i < 8; ++i) {
        this->corner[i] = i * 3;
    }
    for (int i = 0; i < 12; ++i) {
        this->edge[i] = i << 1;
    }
    this->_placement = 0;
}


void Cube::save_to(ostream& out) const {
    char data[21];
    for (int i = 0; i < 8; ++i) {
        data[i] = this->corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        data[i + 8] = this->edge[i];
    }
    data[20] = this->_placement;
    out.write(reinterpret_cast<char*>(data), 21);
}

void Cube::read_from(istream& in) {
    char data[21];
    in.read(reinterpret_cast<char*>(data), 21);
    if (in.gcount() != 21) {
        throw CubeStreamError();
    }
    for (int i = 0; i < 8; ++i) {
        this->corner[i] = data[i];
    }
    for (int i = 0; i < 12; ++i) {
        this->edge[i] = data[i + 8];
    }
    this->_placement = data[20];
}


int Cube::compare(const Cube& lhs, const Cube& rhs) noexcept {
    if (lhs._placement != rhs._placement) {
        return lhs._placement - rhs._placement;
    }
    for (int i = 0; i < 8; ++i) {
        if (lhs.corner[i] != rhs.corner[i]) {
            return lhs.corner[i] - rhs.corner[i];
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (lhs.edge[i] != rhs.edge[i]) {
            return lhs.edge[i] - rhs.edge[i];
        }
    }
    return 0;
}


uint32_t Cube::mask() const noexcept {
    uint32_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        if (this->corner[i] != i * 3) {
            mask |= 1 << i;
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (this->edge[i] != i << 1) {
            mask |= 1 << (i + 8);
        }
    }
    if (this->_placement) {
        mask |= 1 << 20;
    }
    return mask;
}


size_t hash<Cube>::operator()(const Cube& cube) const noexcept {
    size_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result = result * 31 + cube.corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        result = result * 31 + cube.edge[i];
    }
    result = result * 31 + cube._placement;
    return result;
}
