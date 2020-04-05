#include <cstdint>
#include <istream>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include "../utils/encoding.hpp"
using std::uint64_t;
using std::uint8_t;
using InsertionFinder::AlgorithmStreamError;
using InsertionFinder::Case;
using InsertionFinder::CaseStreamError;
using InsertionFinder::Cube;
using InsertionFinder::CubeStreamError;
using InsertionFinder::InsertionAlgorithm;
namespace Details = InsertionFinder::Details;


void Case::save_to(std::ostream& out) const {
    this->_state.save_to(out);
    Details::write_varuint(out, this->list.size());
    for (const InsertionAlgorithm& algorithm: this->list) {
        algorithm.save_to(out);
    }
}

void Case::read_from(std::istream& in) {
    try {
        this->_state.read_from(in);
    } catch (const CubeStreamError& e) {
        throw CaseStreamError();
    }
    this->_mask = this->_state.mask();
    this->_has_parity = this->_state.has_parity();
    this->_corner_cycles = this->_state.corner_cycles();
    this->_edge_cycles = this->_state.edge_cycles();
    uint64_t size;
    if (auto x = Details::read_varuint(in)) {
        size = *x;
    } else {
        throw CaseStreamError();
    }
    this->list.resize(size);
    try {
        for (InsertionAlgorithm& algorithm: this->list) {
            algorithm.read_from(in);
        }
    } catch (const AlgorithmStreamError& e) {
        throw CaseStreamError();
    }
}


int Case::compare(const Case& lhs, const Case& rhs) noexcept {
    int lhs_rotation = lhs.rotation();
    int rhs_rotation = rhs.rotation();
    int lhs_center_cycles = Cube::center_cycles[lhs_rotation];
    int rhs_center_cycles = Cube::center_cycles[rhs_rotation];
    if (
        int cycles_diff =
            (lhs_center_cycles
                + (lhs_center_cycles > 1 ? 0 : lhs._has_parity)
                + lhs._corner_cycles + lhs._edge_cycles)
            - (rhs_center_cycles
                + (rhs_center_cycles > 1 ? 0 : rhs._has_parity)
                + rhs._corner_cycles + rhs._edge_cycles)
    ) {
        return cycles_diff;
    }
    if (int x = lhs_center_cycles - rhs_center_cycles) {
        return x;
    }
    if (int x = lhs_rotation - rhs_rotation) {
        return x;
    }
    if (int x = lhs._has_parity - rhs._has_parity) {
        return x;
    }
    if (int x = lhs._corner_cycles - rhs._corner_cycles) {
        return x;
    }
    return Cube::compare(lhs._state, rhs._state);
}
