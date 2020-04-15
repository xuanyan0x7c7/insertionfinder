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
    this->state.save_to(out);
    Details::write_varuint(out, this->list.size());
    for (const InsertionAlgorithm& algorithm: this->list) {
        algorithm.save_to(out);
    }
}

void Case::read_from(std::istream& in) {
    try {
        this->state.read_from(in);
    } catch (const CubeStreamError& e) {
        throw CaseStreamError();
    }
    this->mask = this->state.mask();
    this->parity = this->state.has_parity();
    this->corner_cycles = this->state.corner_cycles();
    this->edge_cycles = this->state.edge_cycles();
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
    int lhs_placement = lhs.get_placement();
    int rhs_placement = rhs.get_placement();
    int lhs_center_cycles = Cube::center_cycles[lhs_placement];
    int rhs_center_cycles = Cube::center_cycles[rhs_placement];
    if (
        int cycles_diff =
            (lhs_center_cycles + (lhs_center_cycles > 1 ? 0 : lhs.parity) + lhs.corner_cycles + lhs.edge_cycles)
            - (rhs_center_cycles + (rhs_center_cycles > 1 ? 0 : rhs.parity) + rhs.corner_cycles + rhs.edge_cycles)
    ) {
        return cycles_diff;
    }
    if (int x = lhs_center_cycles - rhs_center_cycles) {
        return x;
    }
    if (int x = lhs_placement - rhs_placement) {
        return x;
    }
    if (int x = lhs.parity - rhs.parity) {
        return x;
    }
    if (int x = lhs.corner_cycles - rhs.corner_cycles) {
        return x;
    }
    return Cube::compare(lhs.state, rhs.state);
}
