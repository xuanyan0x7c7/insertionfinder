#include <cstdint>
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
