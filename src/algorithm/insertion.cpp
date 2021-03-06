#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>
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
    int length = this->length();
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
    uint32_t mask_before = 0;
    uint32_t mask_after = 0;
    if (insert_place > 0) {
        mask_before = Details::twist_mask(this->twists[insert_place - 1]);
        if (insert_place > 1 && this->twists[insert_place - 1] >> 3 == this->twists[insert_place - 2] >> 3) {
            mask_before |= Details::twist_mask(this->twists[insert_place - 2]);
        }
    }
    if (insert_place < this->length()) {
        mask_after = Details::twist_mask(this->twists[insert_place]);
        if (
            insert_place + 1 < this->length()
            && this->twists[insert_place] >> 3 == this->twists[insert_place + 1] >> 3
        ) {
            mask_after |= Details::twist_mask(this->twists[insert_place + 1]);
        }
    }
    return {mask_before, mask_after};
}


std::pair<Algorithm, size_t> Algorithm::insert(const Algorithm& insertion, size_t insert_place) const {
    Algorithm result;
    result.twists.reserve(this->length() + insertion.length());
    result.twists.assign(this->twists.cbegin(), this->twists.cbegin() + insert_place);
    result.twists.insert(result.twists.cend(), insertion.twists.cbegin(), insertion.twists.cend());
    if (insertion.rotation == 0) {
        result.twists.insert(result.twists.cend(), this->twists.cbegin() + insert_place, this->twists.cend());
    } else {
        for (size_t index = insert_place; index < this->length(); ++index) {
            result.twists.push_back(this->twists[index] / insertion.rotation);
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
    algorithm.twists.reserve(this->length() + insertion.length());
    algorithm.twists.assign(this->twists.cbegin(), this->twists.cbegin() + insert_place);
    algorithm.twists.insert(algorithm.twists.cend(), insertion.twists.cbegin(), insertion.twists.cend());
    if (insertion.rotation == 0) {
        algorithm.twists.insert(algorithm.twists.cend(), this->twists.cbegin() + insert_place, this->twists.cend());
    } else {
        for (size_t index = insert_place; index < this->length(); ++index) {
            algorithm.twists.push_back(this->twists[index] / insertion.rotation);
        }
    }
    auto marks = algorithm.cancel_moves_return_marks();
    skeleton_marks.reserve(this->length());
    skeleton_marks.assign(marks.cbegin(), marks.cbegin() + insert_place);
    skeleton_marks.insert(skeleton_marks.cend(), marks.cbegin() + insert_place + insertion.length(), marks.cend());
    insertion_marks.assign(marks.cbegin() + insert_place, marks.cbegin() + insert_place + insertion.length());
    return result;
}

std::tuple<Algorithm, std::vector<int>, std::vector<std::vector<int>>>
Algorithm::multi_insert_return_marks(
    const std::vector<Algorithm>& insertions,
    const std::vector<std::pair<size_t, std::vector<size_t>>>& insert_places
) const {
    std::tuple<Algorithm, std::vector<int>, std::vector<std::vector<int>>> result;
    Algorithm& algorithm = std::get<0>(result);
    auto& skeleton_marks = std::get<1>(result);
    auto& insertion_marks = std::get<2>(result);
    skeleton_marks.resize(this->length());
    insertion_marks.resize(insertions.size());
    std::vector<std::pair<int, std::size_t>> table;
    Rotation rotation = 0;
    for (size_t i = 0; i < insert_places.size(); ++i) {
        for (size_t j = (i == 0 ? 0 : insert_places[i - 1].first); j < insert_places[i].first; ++j) {
            table.emplace_back(-1, j);
            algorithm.twists.push_back(this->twists[j] / rotation);
        }
        for (size_t index: insert_places[i].second) {
            const Algorithm& insertion = insertions[index];
            insertion_marks[index].resize(insertion.length());
            for (size_t j = 0; j < insertion.length(); ++j) {
                table.emplace_back(index, j);
                algorithm.twists.push_back(insertion.twists[j] / rotation);
            }
            rotation *= insertion.rotation;
        }
    }
    for (size_t j = insert_places.back().first; j < this->length(); ++j) {
        table.emplace_back(-1, j);
        algorithm.twists.push_back(this->twists[j] / rotation);
    }
    auto marks = algorithm.cancel_moves_return_marks();
    for (size_t i = 0; i < marks.size(); ++i) {
        if (table[i].first == -1) {
            skeleton_marks[table[i].second] = marks[i];
        } else {
            insertion_marks[table[i].first][table[i].second] = marks[i];
        }
    }
    return result;
}


bool Algorithm::is_worthy_insertion(
    const InsertionAlgorithm& insertion, size_t insert_place,
    std::pair<uint32_t, uint32_t> insert_place_mask,
    size_t fewest_twists
) const {
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
                cancellation += (this->length() - insert_place) << 1;
            } else {
                cancellation += 3;
            }
        } else if (low_mask) {
            cancellation += (this->length() - insert_place) << 1;
        } else {
            ++cancellation;
        }
    }
    return fewest_twists == std::numeric_limits<size_t>::max()
        || this->length() + insertion.length() <= fewest_twists + cancellation;
}
