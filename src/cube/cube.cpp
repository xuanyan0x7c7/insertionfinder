#include <cstdint>
#include <array>
#include <istream>
#include <functional>
#include <memory>
#include <ostream>
#include <vector>
#include <insertionfinder/cube.hpp>
using namespace std;
using namespace InsertionFinder;


array<Cube, 24> Cube::twist_cube = {};
array<Cube, 24> Cube::rotation_cube = {};
vector<array<int, 24>> Cube::corner_cycle_transform(6 * 24 * 24);
vector<array<int, 24>> Cube::edge_cycle_transform(10 * 24 * 24);

void Cube::init() {
    Cube::generate_twist_cube_table();
    Cube::generate_rotation_cube_table();
    Cube::generate_corner_cycle_transform_table();
    Cube::generate_edge_cycle_transform_table();
}


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
    out.write(data, 21);
}

void Cube::read_from(istream& in) {
    char data[21];
    in.read(data, 21);
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
    result._placement = Cube::inverse_center[this->_placement];
    return result;
}


uint32_t Cube::mask() const noexcept {
    static constexpr uint32_t center_mask[4] = {0, 3, 12, 15};
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
    mask |= center_mask[Cube::center_cycles[this->_placement]] << 20;
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
