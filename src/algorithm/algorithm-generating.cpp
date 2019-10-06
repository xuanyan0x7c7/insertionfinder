#include <cstdint>
#include <algorithm>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


void Algorithm::normalize() noexcept {
    for (size_t i = 1, length = this->twists.size(); i < length; ++i) {
        if (this->swappable(i) && this->twists[i - 1] > this->twists[i]) {
            this->swap_adjacent(i++);
        }
    }
}


void Algorithm::rotate(int rotation) {
    const uint_fast8_t* table = rotation_permutation[rotation];
    for (uint_fast8_t& twist: this->twists) {
        twist = transform_twist(table, twist);
    }
}


vector<Algorithm> Algorithm::generate_isomorphisms() const {
    vector<Algorithm> result(96);
    size_t length = this->twists.size();
    for (size_t i = 0; i < 96; ++i) {
        result[i].twists.resize(length);
    }
    for (size_t i = 0; i < 24; ++i) {
        const uint_fast8_t* table = rotation_permutation[i];
        for (size_t index = 0; index < length; ++index) {
            size_t inversed_index = length - 1 - index;
            uint_fast8_t twist = this->twists[index];
            uint_fast8_t result_twist = transform_twist(table, twist);
            result[i].twists[index] = result_twist;
            result[i + 24].twists[inversed_index] = Algorithm::inverse_twist[result_twist];
            result[i + 48].twists[index] = result_twist < 16
                ? Algorithm::inverse_twist[result_twist]
                : 40 - result_twist;
            result[i + 72].twists[inversed_index] = Algorithm::inverse_twist[result[i + 48].twists[index]];
        }
    }
    for (Algorithm& algorithm: result) {
        algorithm.normalize();
        algorithm.detect_rotation();
    }
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    return result;
}
