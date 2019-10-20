#include <cstdint>
#include <algorithm>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint_fast16_t;
using InsertionFinder::Algorithm;
namespace Details = InsertionFinder::Details;


std::vector<Algorithm> Algorithm::generate_isomorphisms() const {
    std::vector<Algorithm> result(96);
    size_t length = this->twists.size();
    for (size_t i = 0; i < 96; ++i) {
        result[i].twists.resize(length);
    }
    for (size_t i = 0; i < 24; ++i) {
        const uint_fast8_t* table = Details::rotation_permutation[i];
        for (size_t index = 0; index < length; ++index) {
            size_t inversed_index = length - 1 - index;
            uint_fast8_t twist = this->twists[index];
            uint_fast8_t result_twist = Details::transform_twist(table, twist);
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
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}
