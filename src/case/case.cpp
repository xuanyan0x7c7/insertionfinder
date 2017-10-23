#include <cstdint>
#include <istream>
#include <ostream>
#include <algorithm.hpp>
#include <cube.hpp>
#include <case.hpp>
using namespace std;
using namespace InsertionFinder;


Case::Case(const Cube& state):
    state(state),
    mask(state.mask()),
    has_parity(state.has_parity()),
    corner_cycles(state.corner_cycles()), edge_cycles(state.edge_cycles())
    {}


void Case::save_to(ostream& out) const {
    this->state.save_to(out);
    size_t size = this->algorithm_list.size();
    out.write(reinterpret_cast<char*>(&size), sizeof(size_t));
    for (const auto& algorithm: this->algorithm_list) {
        algorithm.save_to(out);
    }
}

void Case::read_from(istream& in) {
    try {
        this->state.read_from(in);
    } catch (const CubeStreamError& e) {
        throw CaseStreamError();
    }
    this->mask = this->state.mask();
    this->has_parity = this->state.has_parity();
    this->corner_cycles = this->state.corner_cycles();
    this->edge_cycles = this->state.edge_cycles();
    size_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
    if (in.gcount() != sizeof(size_t)) {
        throw CaseStreamError();
    }
    this->algorithm_list.resize(size);
    try {
        for (auto& algorithm: this->algorithm_list) {
            algorithm.read_from(in);
        }
    } catch (const AlgorithmStreamError& e) {
        throw CaseStreamError();
    }
}
