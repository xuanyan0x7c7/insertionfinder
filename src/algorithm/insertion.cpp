#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <utility>
#include <range/v3/all.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint32_t;
using std::uint_fast8_t;
using InsertionFinder::Algorithm;
namespace Details = InsertionFinder::Details;


size_t Algorithm::cancel_moves() {
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
                uint_fast8_t orientation = (*(p - 1) + *q) & 3;
                if (orientation == 0) {
                    *(p - 1) = *p;
                    --p;
                } else {
                    *(p - 1) = (*(p - 1) & ~3) | orientation;
                }
            } else {
                *++p = *q;
            }
        } else {
            uint_fast8_t orientation = (*p + *q) & 3;
            if (orientation == 0) {
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
    result.twists.insert(result.twists.end(), insertion.twists.cbegin(), insertion.twists.cend());
    ranges::move(
        this->twists
            | ranges::views::slice(insert_place, ranges::end)
            | ranges::views::transform(
                std::bind(
                    Details::transform_twist,
                    Details::rotation_permutation[Cube::inverse_center[insertion.rotation]],
                    std::placeholders::_1
                )
            ),
        ranges::back_inserter(result.twists)
    );
    size_t place = result.cancel_moves();
    return {result, std::min(place, insert_place + 1)};
}


bool Algorithm::is_worthy_insertion(
    const Algorithm& insertion, size_t insert_place,
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
