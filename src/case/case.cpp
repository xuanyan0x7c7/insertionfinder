#include <cstdint>
#include <istream>
#include <ostream>
#include <algorithm.hpp>
#include <cube.hpp>
#include <case.hpp>
using namespace std;
using namespace InsertionFinder;


Case::Case(const Cube& state):
    _state(state),
    _mask(state.mask()),
    _has_parity(state.has_parity()),
    _corner_cycles(state.corner_cycles()), _edge_cycles(state.edge_cycles())
    {}


void Case::save_to(ostream& out) const {
    this->_state.save_to(out);
    size_t size = this->list.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(size_t));
    for (const auto& algorithm: this->list) {
        algorithm.save_to(out);
    }
}

void Case::read_from(istream& in) {
    try {
        this->_state.read_from(in);
    } catch (const CubeStreamError& e) {
        throw CaseStreamError();
    }
    this->_mask = this->_state.mask();
    this->_has_parity = this->_state.has_parity();
    this->_corner_cycles = this->_state.corner_cycles();
    this->_edge_cycles = this->_state.edge_cycles();
    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
    if (in.gcount() != sizeof(size_t)) {
        throw CaseStreamError();
    }
    this->list.resize(size);
    try {
        for (auto& algorithm: this->list) {
            algorithm.read_from(in);
        }
    } catch (const AlgorithmStreamError& e) {
        throw CaseStreamError();
    }
}
