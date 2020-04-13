#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/twist.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint_fast8_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Rotation;
using InsertionFinder::Twist;
namespace Details = InsertionFinder::Details;


namespace {
    inline std::pair<size_t, uint_fast8_t> extract_twist(Twist twist) {
        return {twist >> 2 & 1, twist & 3};
    }
}


std::vector<Algorithm> Algorithm::generate_symmetrics() const {
    std::vector<Algorithm> result(96);
    for (size_t i = 0; i < 96; ++i) {
        result[i].twists.resize(this->length());
    }
    for (size_t rotation = 0; rotation < 24; ++rotation) {
        for (size_t index = 0; index < this->length(); ++index) {
            size_t inversed_index = this->length() - 1 - index;
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
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

std::vector<Algorithm> Algorithm::generate_rotation_conjugates() const {
    std::vector<Algorithm> result;
    result.push_back(*this);
    result.front().cancel_moves();
    result.front().normalize();
    size_t length = result.front().length();
    result.reserve(length << 2);
    const Algorithm& base = result.front();
    bool has_next = true;
    for (size_t offset = 1; offset < length; ++offset) {
        Algorithm algorithm;
        algorithm.rotation = base.rotation;
        algorithm.twists.reserve(length);
        algorithm.twists.assign(base.twists.cbegin() + offset, base.twists.cend());
        for (size_t index = 0; index < offset; ++index) {
            algorithm.twists.push_back(base.twists[index] / base.rotation);
        }
        algorithm.cancel_moves();
        if (algorithm.length() == length) {
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
            for (size_t index = 0; index < offset - 1; ++index) {
                swapped_algorithm.twists.push_back(base.twists[index] / base.rotation);
            }
            swapped_algorithm.twists.push_back(base.twists[offset] / base.rotation);
            swapped_algorithm.cancel_moves();
            if (swapped_algorithm.length() == length) {
                swapped_algorithm.normalize();
                result.push_back(std::move(swapped_algorithm));
            } else {
                has_next = false;
            }
        }
    }
    if (length >= 3 && base.twists.front() >> 3 == (base.twists.back() * base.rotation) >> 3) {
        if (has_next) {
            Algorithm algorithm = base;
            algorithm.twists.front() = base.twists.back() * base.rotation;
            algorithm.twists.back() = base.twists.front() * base.rotation.inverse();
            result.push_back(std::move(algorithm));
        } else {
            uint_fast8_t twist_base = (base.twists.front() >> 3) << 3;
            uint_fast8_t sum[2] = {0, 0};
            size_t interval[2] = {1, length - 1};
            auto [index1, offset1] = extract_twist(base.twists[0]);
            sum[index1] = (sum[index1] + offset1) & 3;
            if (base.twists[0] >> 3 == base.twists[1] >> 3) {
                interval[0] = 2;
                auto [index, offset] = extract_twist(base.twists[1]);
                sum[index] = (sum[index] + offset) & 3;
            }
            auto [index2, offset2] = extract_twist(base.twists[length - 1] * base.rotation);
            sum[index2] = (sum[index2] + offset2) & 3;
            if (base.twists[length - 2] >> 3 == base.twists[length - 1] >> 3) {
                interval[1] = length - 2;
                auto [index, offset] = extract_twist(base.twists[length - 2] * base.rotation);
                sum[index] = (sum[index] + offset) & 3;
            }
            size_t holes = length + interval[0] - interval[1];
            for (uint_fast8_t i = 0; i < 4; ++i) {
                for (uint_fast8_t j = 0; j < 4; ++j) {
                    if (holes == (i != 0) + (i != sum[0]) + (j != 0) + (j != sum[1])) {
                        Algorithm algorithm;
                        algorithm.rotation = base.rotation;
                        algorithm.twists.reserve(length);
                        if (i != 0) {
                            algorithm.twists.push_back(twist_base | i);
                        }
                        if (j != 0) {
                            algorithm.twists.push_back(twist_base | 4 | j);
                        }
                        algorithm.twists.insert(
                            algorithm.twists.cend(),
                            base.twists.cbegin() + interval[0],
                            base.twists.cbegin() + interval[1]
                        );
                        if (i != sum[0]) {
                            algorithm.twists.push_back(Twist(twist_base | (sum[0] - i & 3)) / base.rotation);
                        }
                        if (j != sum[1]) {
                            algorithm.twists.push_back(
                                Twist(twist_base | 4 | (sum[1] - j & 3)) / base.rotation
                            );
                        }
                        result.push_back(std::move(algorithm));
                    }
                }
            }
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

std::vector<Algorithm> Algorithm::generate_similars() const {
    std::vector<Algorithm> result;
    for (const Algorithm& algorithm: this->generate_rotation_conjugates()) {
        for (Algorithm& alg: algorithm.generate_symmetrics()) {
            result.push_back(std::move(alg));
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}
