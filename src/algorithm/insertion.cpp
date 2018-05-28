#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <utility>
#include <algorithm.hpp>
#include <cube.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


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
                int orientation = (*(p - 1) + *q) & 3;
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
            int orientation = (*p + *q) & 3;
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


pair<uint32_t, uint32_t>
Algorithm::get_insert_place_mask(size_t insert_place) const {
    const auto& twists = this->twists;
    uint32_t mask_before;
    uint32_t mask_after;
    if (insert_place == 0) {
        mask_before = 0;
    } else {
        mask_before = twist_mask(twists[insert_place - 1]);
        if (
            insert_place > 1
            && twists[insert_place - 1] >> 3 == twists[insert_place - 2] >> 3
        ) {
            mask_before |= twist_mask(twists[insert_place - 2]);
        }
    }
    if (insert_place == twists.size()) {
        mask_after = 0;
    } else {
        mask_after = twist_mask(twists[insert_place]);
        if (
            insert_place + 1 < twists.size()
            && twists[insert_place] >> 3 == twists[insert_place + 1] >> 3
        ) {
            mask_after |= twist_mask(twists[insert_place + 1]);
        }
    }
    return make_pair(mask_before, mask_after);
}


pair<Algorithm, size_t>
Algorithm::insert(const Algorithm& insertion, size_t insert_place) const {
    using namespace placeholders;
    Algorithm result;
    result.twists.reserve(this->twists.size() + insertion.twists.size());
    result.twists.insert(
        result.twists.end(),
        this->twists.cbegin(), this->twists.cbegin() + insert_place
    );
    result.twists.insert(
        result.twists.end(),
        insertion.twists.cbegin(), insertion.twists.cend()
    );
    transform(
        this->twists.cbegin() + insert_place, this->twists.cend(),
        back_inserter(result.twists),
        bind(
            transform_twist,
            rotation_permutation[Cube::inverse_center[insertion.rotation]],
            _1
        )
    );
    size_t place = result.cancel_moves();
    return {result, min(place, insert_place + 1)};
}


bool Algorithm::is_worthy_insertion(
    const Algorithm& insertion, size_t insert_place,
    const pair<uint32_t, uint32_t>& insert_place_mask,
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
    return fewest_twists == numeric_limits<size_t>::max()
        || length + insertion.twists.size() <= fewest_twists + cancellation;
}
