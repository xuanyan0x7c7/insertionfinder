#include <cstring>
#include <chrono>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
using std::size_t;
using InsertionFinder::Case;
using InsertionFinder::Cube;
using InsertionFinder::Finder;
namespace FinderStatus = InsertionFinder::FinderStatus;


void Finder::init() {
    std::memset(this->corner_cycle_index, 0xff, sizeof(this->corner_cycle_index));
    std::memset(this->edge_cycle_index, 0xff, sizeof(this->edge_cycle_index));
    std::memset(this->center_index, 0xff, sizeof(this->center_index));

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
}

void Finder::search(const SearchParams& params) {
    this->fewest_moves = params.search_target;
    this->parity_multiplier = params.parity_multiplier * 2;
    auto begin = std::chrono::high_resolution_clock::now();
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
        std::sort(
            this->solutions.begin(), this->solutions.end(),
            [](const auto& x, const auto& y) {return x.cancellation < y.cancellation;}
        );
    }
    auto end = std::chrono::high_resolution_clock::now();
    this->result.duration = (end - begin).count();
}
