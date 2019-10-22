#include <cstdint>
#include <vector>
#include <range/v3/all.hpp>
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
    for (size_t rotation = 0; rotation < 24; ++rotation) {
        for (size_t index = 0; index < length; ++index) {
            size_t inversed_index = length - 1 - index;
            uint_fast8_t twist = this->twists[index];
            uint_fast8_t result_twist = Details::rotate_twist(rotation, twist);
            result[rotation].twists[index] = result_twist;
            result[rotation + 24].twists[inversed_index] = Algorithm::inverse_twist[result_twist];
            uint_fast8_t twist2 = result_twist < 16 ? Algorithm::inverse_twist[result_twist] : 40 - result_twist;
            result[rotation + 48].twists[index] = twist2;
            result[rotation + 72].twists[inversed_index] = Algorithm::inverse_twist[twist2];
        }
    }
    for (Algorithm& algorithm: result) {
        algorithm.normalize();
        algorithm.detect_rotation();
    }
    result |= ranges::actions::sort | ranges::actions::unique;
    return result;
}
