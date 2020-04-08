#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>
#include <range/v3/all.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint32_t;
using std::uint_fast8_t;
using InsertionFinder::Algorithm;
using InsertionFinder::InsertionAlgorithm;
namespace Details = InsertionFinder::Details;


size_t Algorithm::cancel_moves() noexcept {
    auto begin = this->twists.begin();
    auto end = this->twists.cend();
    auto p = begin - 1;
    auto needle = end;
    for (auto q = begin; q != end; ++q) {
        if (p < begin || *p >> 3 != *q >> 3) {
            *++p = *q;
        } else if (*p >> 2 != *q >> 2) {
            if (p > begin && *(p - 1) >> 3 == *p >> 3) {
                if (needle > p) {
                    needle = p;
                }
                if (uint_fast8_t orientation = (*(p - 1) + *q) & 3; orientation == 0) {
                    *(p - 1) = *p;
                    --p;
                } else {
                    *(p - 1) = (*(p - 1) & ~3) | orientation;
                }
            } else {
                *++p = *q;
            }
        } else {
            if (uint_fast8_t orientation = (*p + *q) & 3; orientation == 0) {
                --p;
            } else {
                *p = (*p & ~3) | orientation;
            }
            if (needle > p + 1) {
                needle = p + 1;
            }
        }
    }
    this->twists.resize(p + 1 - begin);
    return needle - begin;
}

std::vector<int> Algorithm::cancel_moves_return_marks() {
    auto& twists = this->twists;
    int length = twists.size();
    std::vector<std::vector<int>> stacks(length);
    int y = -1;
    for (int x = 0; x < length; ++x) {
        if (y < 0 || twists[y] >> 3 != twists[x] >> 3) {
            twists[++y] = twists[x];
            stacks[y].push_back(x);
        } else if (twists[y] >> 2 != twists[x] >> 2) {
            if (y > 0 && twists[y - 1] >> 3 == twists[y] >> 3) {
                if (uint_fast8_t orientation = (twists[y - 1] + twists[x]) & 3; orientation == 0) {
                    twists[y - 1] = twists[y];
                    stacks[y - 1].swap(stacks[y]);
                    stacks[y--].clear();
                } else {
                    twists[y - 1] = (twists[y - 1] & ~3) | orientation;
                    stacks[y - 1].push_back(x);
                }
            } else {
                twists[++y] = twists[x];
                stacks[y].push_back(x);
            }
        } else {
            if (uint_fast8_t orientation = (twists[y] + twists[x]) & 3; orientation == 0) {
                stacks[y--].clear();
            } else {
                twists[y] = (twists[y] & ~3) | orientation;
                stacks[y].push_back(x);
            }
        }
    }
    this->twists.resize(y + 1);
    std::vector<int> marks(length, 2);
    for (const auto& list: stacks) {
        if (list.size() == 1) {
            marks[list.front()] = 0;
        } else {
            for (int x: list) {
                marks[x] = 1;
            }
        }
    }
    return marks;
}


std::pair<uint32_t, uint32_t> Algorithm::get_insert_place_mask(size_t insert_place) const {
    const auto& twists = this->twists;
    uint32_t mask_before = 0;
    uint32_t mask_after = 0;
    if (insert_place > 0) {
        mask_before = Details::twist_mask(twists[insert_place - 1]);
        if (insert_place > 1 && twists[insert_place - 1] >> 3 == twists[insert_place - 2] >> 3) {
            mask_before |= Details::twist_mask(twists[insert_place - 2]);
        }
    }
    if (insert_place < twists.size()) {
        mask_after = Details::twist_mask(twists[insert_place]);
        if (insert_place + 1 < twists.size() && twists[insert_place] >> 3 == twists[insert_place + 1] >> 3) {
            mask_after |= Details::twist_mask(twists[insert_place + 1]);
        }
    }
    return {mask_before, mask_after};
}


std::pair<Algorithm, size_t> Algorithm::insert(const Algorithm& insertion, size_t insert_place) const {
    Algorithm result;
    result.twists.reserve(this->twists.size() + insertion.twists.size());
    result.twists.assign(this->twists.cbegin(), this->twists.cbegin() + insert_place);
    result.twists.insert(result.twists.cend(), insertion.twists.cbegin(), insertion.twists.cend());
    if (insertion.rotation == 0) {
        result.twists.insert(result.twists.cend(), this->twists.cbegin() + insert_place, this->twists.cend());
    } else {
        for (Twist twist: ranges::views::slice(this->twists, insert_place, ranges::end)) {
            result.twists.push_back(twist * insertion.rotation.inverse());
        }
    }
    size_t place = result.cancel_moves();
    return {result, std::min(place, insert_place + 1)};
}

std::tuple<Algorithm, std::vector<int>, std::vector<int>>
Algorithm::insert_return_marks(const Algorithm& insertion, size_t insert_place) const {
    std::tuple<Algorithm, std::vector<int>, std::vector<int>> result;
    Algorithm& algorithm = std::get<0>(result);
    auto& skeleton_marks = std::get<1>(result);
    auto& insertion_marks = std::get<2>(result);
    algorithm.twists.reserve(this->twists.size() + insertion.twists.size());
    algorithm.twists.assign(this->twists.cbegin(), this->twists.cbegin() + insert_place);
    algorithm.twists.insert(algorithm.twists.cend(), insertion.twists.cbegin(), insertion.twists.cend());
    if (insertion.rotation == 0) {
        algorithm.twists.insert(algorithm.twists.cend(), this->twists.cbegin() + insert_place, this->twists.cend());
    } else {
        for (Twist twist: ranges::views::slice(this->twists, insert_place, ranges::end)) {
            algorithm.twists.push_back(twist * insertion.rotation.inverse());
        }
    }
    auto marks = algorithm.cancel_moves_return_marks();
    skeleton_marks.reserve(this->twists.size());
    skeleton_marks.assign(marks.cbegin(), marks.cbegin() + insert_place);
    skeleton_marks.insert(skeleton_marks.cend(), marks.cbegin() + insert_place + insertion.twists.size(), marks.cend());
    insertion_marks.assign(marks.cbegin() + insert_place, marks.cbegin() + insert_place + insertion.twists.size());
    return result;
}


bool Algorithm::is_worthy_insertion(
    const InsertionAlgorithm& insertion, size_t insert_place,
    std::pair<uint32_t, uint32_t> insert_place_mask,
    size_t fewest_twists
) const {
    size_t length = this->twists.size();
    size_t cancellation = 0;
    if (uint32_t begin_mask = insert_place_mask.first & insertion.begin_mask) {
        if (begin_mask & (insertion.set_up_mask | 0xffffff)) {
            return false;
        }
        uint32_t high_mask = begin_mask >> 24;
        cancellation += high_mask & (high_mask - 1) ? 2 : 1;
    }
    if (uint32_t end_mask = insert_place_mask.second & insertion.end_mask) {
        if (end_mask & insertion.set_up_mask & 0xffffff) {
            return false;
        }
        uint32_t low_mask = end_mask & 0xffffff;
        uint32_t high_mask = end_mask >> 24;
        if (high_mask & (high_mask - 1)) {
            if (low_mask == 0) {
                cancellation += 2;
            } else if (low_mask & (low_mask - 1)) {
                cancellation += (length - insert_place) << 1;
            } else {
                cancellation += 3;
            }
        } else if (low_mask) {
            cancellation += (length - insert_place) << 1;
        } else {
            ++cancellation;
        }
    }
    return fewest_twists == std::numeric_limits<size_t>::max()
        || length + insertion.twists.size() <= fewest_twists + cancellation;
}
