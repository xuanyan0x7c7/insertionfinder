#include <chrono>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
using namespace std;
using namespace chrono;
using namespace InsertionFinder;


void Finder::init() {
    this->corner_cycle_index.fill(-1);
    this->edge_cycle_index.fill(-1);
    this->center_index.fill(-1);

    for (size_t index = 0; index < this->cases.size(); ++index) {
        const Case& _case = this->cases[index];
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
        if (parity || Cube::center_cycles[rotation] > 1) {
            this->change_parity = true;
            this->result.status &= ~FinderStatus::parity_algorithms_needed;
        }
        if (corner_changed) {
            this->change_corner = true;
            this->result.status &= ~FinderStatus::corner_cycle_algorithms_needed;
        }
        if (edge_changed) {
            this->change_edge = true;
            this->result.status &= ~FinderStatus::edge_cycle_algorithms_needed;
        }
        if (rotation) {
            this->change_center = true;
            this->result.status &= ~FinderStatus::center_algorithms_needed;
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
    this->inverse_scramble_cube = Cube::inverse(this->scramble_cube);
}

void Finder::search(const SearchParams& params) {
    this->fewest_moves = params.search_target;
    this->parity_multiplier = params.parity_multiplier * 2;
    auto begin = high_resolution_clock::now();
    this->search_core(params);
    if (this->result.status == FinderStatus::success) {
        for (auto& solution: this->solutions) {
            size_t cancellation = solution.insertions.front().skeleton.length();
            for (size_t i = 0; i < solution.insertions.size() - 1; ++i) {
                cancellation += solution.insertions[i].insertion->length();
            }
            cancellation -= solution.insertions.back().skeleton.length();
            solution.cancellation = cancellation;
        }
        sort(
            this->solutions.begin(), this->solutions.end(),
            [](const auto& x, const auto& y) {return x.cancellation < y.cancellation;}
        );
    }
    auto end = high_resolution_clock::now();
    this->result.duration = (end - begin).count();
}
