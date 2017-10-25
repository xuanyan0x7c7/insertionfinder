#include <cstdint>
#include <istream>
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
}


void Cube::save_to(ostream& out) const {
    int8_t data[20];
    for (int i = 0; i < 8; ++i) {
        data[i] = this->corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        data[i + 8] = this->edge[i];
    }
    out.write(reinterpret_cast<char*>(data), 20);
}

void Cube::read_from(istream& in) {
    int8_t data[20];
    in.read(reinterpret_cast<char*>(data), 20);
    if (in.gcount() != 20) {
        throw CubeStreamError();
    }
    for (int i = 0; i < 8; ++i) {
        this->corner[i] = data[i];
    }
    for (int i = 0; i < 12; ++i) {
        this->edge[i] = data[i + 8];
    }
}


Cube Cube::inverse() const noexcept {
    Cube result(nullopt);
    for (int i = 0; i < 8; ++i) {
        int item = this->corner[i];
        result.corner[item / 3] = i * 3 + (24 - item) % 3;
    }
    for (int i = 0; i < 12; ++i) {
        int item = this->edge[i];
        result.edge[item >> 1] = i << 1 | (item & 1);
    }
    return result;
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
    return mask;
}
