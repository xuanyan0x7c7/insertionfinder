#include <array>
#include <chrono>
#include <limits>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
using namespace std;
using namespace chrono;
using namespace InsertionFinder;


Finder::Finder(
    const Algorithm& scramble, const Algorithm& skeleton,
    const vector<Case>& cases
):
    scramble(scramble), skeleton(skeleton), cases(cases),
    fewest_moves(numeric_limits<size_t>::max()),
    verbose(false),
    change_parity(false),
    change_corner(false), change_edge(false),
    change_center(false) {
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


void Finder::search(const SearchParams& params) {
    auto begin = high_resolution_clock::now();

    do {
        Cube original_cube = this->scramble_cube;
        original_cube.twist(this->skeleton);
        Cube cube = original_cube.best_placement();
        bool parity = cube.has_parity();
        int corner_cycles = cube.corner_cycles();
        int edge_cycles = cube.edge_cycles();
        int placement = cube.placement();
        if (!parity && corner_cycles == 0 && edge_cycles == 0 && placement == 0) {
            this->result.status = Finder::Status::SUCCESS_SOLVED;
            break;
        } else if ((parity || Cube::center_cycles[placement] > 1) && !this->change_parity) {
            this->result.status = Finder::Status::FAILURE_PARITY_ALGORITHMS_NEEDED;
            break;
        } else if (corner_cycles && !this->change_corner) {
            this->result.status = Finder::Status::FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED;
            break;
        } else if (edge_cycles && !this->change_edge) {
            this->result.status = Finder::Status::FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED;
            break;
        } else if (placement && !this->change_center) {
            this->result.status = Finder::Status::FAILURE_CENTER_ALGORITHMS_NEEDED;
            break;
        }

        this->search_core({parity, corner_cycles, edge_cycles, placement}, params);
        this->result.status = Finder::Status::SUCCESS;

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
    } while (false);

    auto end = high_resolution_clock::now();
    this->result.duration = (end - begin).count();
}
