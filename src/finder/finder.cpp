#include <array>
#include <chrono>
#include <limits>
#include <vector>
#include <algorithm.hpp>
#include <case.hpp>
#include <cube.hpp>
#include <finder/finder.hpp>
using namespace std;
using namespace chrono;
using namespace InsertionFinder;


Finder::Finder(
    const Algorithm& scramble, const Algorithm& skeleton,
    const vector<Case>& cases
):
    scramble(scramble), skeleton(skeleton), cases(cases),
    fewest_moves(numeric_limits<size_t>::max()),
    change_parity(false),
    change_corner(false), change_edge(false),
    change_center(false),
    verbose(false) {
    this->parity_index.fill(-1);
    this->corner_cycle_index.fill(-1);
    this->edge_cycle_index.fill(-1);
    this->center_index.fill(-1);
    this->change_parity = false;

    for (size_t index = 0; index < cases.size(); ++index) {
        const Case& _case = cases[index];
        const Cube& state = _case.state();
        if (state.mask() == 0) {
            continue;
        }
        bool parity = _case.has_parity();
        int corner_cycles = _case.corner_cycles();
        int edge_cycles = _case.edge_cycles();
        int rotation = _case.rotation();
        bool corner_changed = _case.mask() & 0xff;
        bool edge_changed = _case.mask() & 0xfff00;
        bool center_chaged = _case.mask() & 0x300000;
        if (parity || Cube::center_cycles[rotation] > 1) {
            this->change_parity = true;
        }
        if (corner_changed) {
            this->change_corner = true;
        }
        if (edge_changed) {
            this->change_edge = true;
        }
        if (rotation) {
            this->change_center = true;
        }
        if (parity && corner_cycles == 0 && edge_cycles == 0 && rotation == 0) {
            this->parity_index[state.parity_index()] = index;
        }
        if (!parity && corner_cycles == 1 && edge_cycles == 0 && rotation == 0) {
            this->corner_cycle_index[state.corner_cycle_index()] = index;
        }
        if (!parity && corner_cycles == 0 && edge_cycles == 1 && rotation == 0) {
            this->edge_cycle_index[state.edge_cycle_index()] = index;
        }
        if (!parity && corner_cycles == 0 && edge_cycles == 0 && Cube::center_cycles[rotation] == 1) {
            this->center_index[rotation] = index;
        }
    }

    this->scramble_cube.twist(scramble);
    this->inverse_scramble_cube = this->scramble_cube.inverse();
}


void Finder::search(size_t max_threads) {
    auto begin = high_resolution_clock::now();
    this->result.status = this->search_core(max_threads);
    auto end = high_resolution_clock::now();
    this->result.duration = (end - begin).count();
}
