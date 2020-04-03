#include <cstddef>
#include <cstdint>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/twist.hpp>
#include <insertionfinder/improver/improver.hpp>
#include <insertionfinder/improver/slice.hpp>
using std::size_t;
using std::uint32_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Cube;
using InsertionFinder::InsertionAlgorithm;
using InsertionFinder::Rotation;
using InsertionFinder::Twist;
using InsertionFinder::SliceImprover;
namespace CubeTwist = InsertionFinder::CubeTwist;


void SliceImprover::Worker::search() {
    static constexpr std::byte twist_flag = CubeTwist::edges;
    Cube state;
    for (size_t insert_place = 0; insert_place <= this->skeleton.length(); ++insert_place) {
        if (insert_place == 0) {
            state.twist(this->skeleton, twist_flag);
            state.rotate(this->placement);
            state.twist(this->improver.inverse_skeleton_cube, twist_flag);
        } else {
            Twist twist = this->skeleton[insert_place - 1];
            state.twist_before(twist.inverse(), twist_flag);
            state.twist(twist, twist_flag);
        }
        this->try_insertion(insert_place, state);

        if (this->skeleton.swappable(insert_place)) {
            Twist twist0 = this->skeleton[insert_place - 1];
            Twist twist1 = this->skeleton[insert_place];
            Cube swapped_state;
            swapped_state.twist(twist0, twist_flag);
            swapped_state.twist(twist1.inverse(), twist_flag);
            swapped_state.twist(state, twist_flag);
            swapped_state.twist(twist1, twist_flag);
            swapped_state.twist(twist0.inverse(), twist_flag);
            this->try_insertion(insert_place, swapped_state, true);
        }
    }
}

void SliceImprover::Worker::try_insertion(std::size_t insert_place, const Cube& state, bool swapped) {
    static constexpr std::byte twist_flag = CubeTwist::edges;
    static constexpr uint32_t valid_masks[3] = {0x3cf0000, 0x3305500, 0x0f0aa00};
    Algorithm skeleton = this->skeleton;
    if (swapped) {
        skeleton.swap_adjacent(insert_place);
    }
    auto insert_place_mask = skeleton.get_insert_place_mask(insert_place);

    for (const Case& _case: this->improver.cases) {
        Cube cube = Cube::twist(state, _case.state(), twist_flag, twist_flag);
        uint32_t mask = cube.mask();
        if ((mask & ~valid_masks[0]) && (mask & ~valid_masks[1]) && (mask & ~valid_masks[2])) {
            continue;
        }
        for (const InsertionAlgorithm& algorithm: _case.algorithm_list()) {
            size_t target = this->improver.fewest_moves + this->improver.options.threshold;
            if (!skeleton.is_worthy_insertion(algorithm, insert_place, insert_place_mask, target)) {
                continue;
            }
            Algorithm new_skeleton = skeleton.insert(algorithm, insert_place).first;
            if (new_skeleton.length() > target) {
                continue;
            }
            new_skeleton.normalize();
            if (cube.mask() == 0) {
                std::lock_guard<std::mutex> lock(this->improver.fewest_moves_mutex);
                if (new_skeleton.length() < this->improver.fewest_moves) {
                    this->improver.fewest_moves = new_skeleton.length();
                    this->improver.partial_solution_list.clear();
                }
                this->improver.partial_solution_list.insert(new_skeleton);
            }
            this->improver.run_worker(
                this->pool,
                std::move(new_skeleton),
                SolvingStep {
                    &this->skeleton, insert_place, &algorithm, swapped, cube.placement(),
                    this->cancellation + this->skeleton.length() + algorithm.length() - new_skeleton.length()
                }
            );
        }
    }
}
