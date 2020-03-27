#include <cstdint>
#include <utility>
#include <vector>
#include <range/v3/all.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/twist.hpp>
#include "utils.hpp"
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Rotation;
using InsertionFinder::Twist;
namespace Details = InsertionFinder::Details;


namespace {
    void swap_twists(std::vector<Twist>& twists, size_t first, size_t last, Rotation rotation) {
        Twist temp = twists[first];
        twists[first] = twists[last] * rotation;
        twists[last] = temp * rotation.inverse();
    }
}


std::vector<Algorithm> Algorithm::generate_symmetrics() const {
    std::vector<Algorithm> result(96);
    size_t length = this->twists.size();
    for (size_t i = 0; i < 96; ++i) {
        result[i].twists.resize(length);
    }
    for (size_t rotation = 0; rotation < 24; ++rotation) {
        for (size_t index = 0; index < length; ++index) {
            size_t inversed_index = length - 1 - index;
            Twist twist = this->twists[index];
            Twist result_twist = twist * Rotation(rotation);
            result[rotation].twists[index] = result_twist;
            result[rotation + 24].twists[inversed_index] = result_twist.inverse();
            Twist twist2 = result_twist < 16 ? result_twist.inverse() : Twist(40 - result_twist);
            result[rotation + 48].twists[index] = twist2;
            result[rotation + 72].twists[inversed_index] = twist2.inverse();
        }
    }
    for (Algorithm& algorithm: result) {
        algorithm.normalize();
        algorithm.detect_rotation();
    }
    result |= ranges::actions::sort | ranges::actions::unique;
    return result;
}

std::vector<Algorithm> Algorithm::generate_rotation_conjugates() const {
    std::vector<Algorithm> result;
    result.push_back(*this);
    result.front().cancel_moves();
    result.front().normalize();
    size_t length = result.front().twists.size();
    result.reserve(length << 1);
    const Algorithm& base = result.front();
    bool has_next = true;
    for (size_t offset = 1; offset < length && has_next; ++offset) {
        Algorithm algorithm;
        algorithm.rotation = base.rotation;
        algorithm.twists.reserve(length);
        algorithm.twists.assign(base.twists.cbegin() + offset, base.twists.cend());
        for (Twist twist: ranges::views::slice(base.twists, 0, offset)) {
            algorithm.twists.push_back(twist * base.rotation.inverse());
        }
        algorithm.cancel_moves();
        if (algorithm.twists.size() == length) {
            algorithm.normalize();
            result.push_back(std::move(algorithm));
        } else {
            has_next = false;
        }
        if (base.swappable(offset)) {
            Algorithm swapped_algorithm;
            swapped_algorithm.rotation = base.rotation;
            swapped_algorithm.twists.reserve(length);
            swapped_algorithm.twists.assign(base.twists.cbegin() + offset, base.twists.cend());
            swapped_algorithm.twists.front() = base.twists[offset - 1];
            for (Twist twist: ranges::views::slice(base.twists, 0, offset - 1)) {
                swapped_algorithm.twists.push_back(twist * base.rotation.inverse());
            }
            swapped_algorithm.twists.push_back(base.twists[offset] * base.rotation.inverse());
            swapped_algorithm.cancel_moves();
            if (swapped_algorithm.twists.size() == length) {
                swapped_algorithm.normalize();
                result.push_back(std::move(swapped_algorithm));
            } else {
                has_next = false;
            }
        }
    }
    if (length >= 3 && base.twists.front() >> 3 == (base.twists.back() * base.rotation) >> 3) {
        bool is_parallel[2] = {
            base.twists[0] >> 3 == base.twists[1] >> 3,
            base.twists[length - 2] >> 3 == base.twists[length - 1] >> 3
        };
        std::vector<Algorithm> algorithms(is_parallel[0] + is_parallel[1] + 1, base);
        if (is_parallel[0]) {
            if (is_parallel[1]) {
                swap_twists(algorithms[0].twists, 0, length - 2, base.rotation);
                swap_twists(algorithms[1].twists, 1, length - 1, base.rotation);
                swap_twists(algorithms[2].twists, 0, length - 2, base.rotation);
                swap_twists(algorithms[2].twists, 1, length - 1, base.rotation);
            } else {
                swap_twists(algorithms[0].twists, 0, length - 1, base.rotation);
                swap_twists(algorithms[1].twists, 1, length - 1, base.rotation);
            }
        } else if (is_parallel[1]) {
            swap_twists(algorithms[0].twists, 0, length - 1, base.rotation);
            swap_twists(algorithms[1].twists, 0, length - 2, base.rotation);
        } else {
            swap_twists(algorithms[0].twists, 0, length - 1, base.rotation);
        }
        for (Algorithm& algorithm: algorithms) {
            algorithm.cancel_moves();
            if (algorithm.twists.size() == length) {
                algorithm.normalize();
                result.push_back(std::move(algorithm));
            }
        }
    }
    result |= ranges::actions::sort | ranges::actions::unique;
    return result;
}
