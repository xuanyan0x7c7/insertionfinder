#include <algorithm>
#include <vector>
#include <algorithm.hpp>
using namespace std;
using namespace InsertionFinder;


void Algorithm::normalize() noexcept {
    for (size_t i = 1, length = this->twists.size(); i < length; ++i) {
        if (this->swappable(i) && this->twists[i - 1] > this->twists[i]) {
            this->swap_adjacent(i++);
        }
    }
}


vector<Algorithm> Algorithm::generate_isomorphisms() const {
    static constexpr int transform_table[24][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
        {4, 0, 2}, {3, 0, 4}, {5, 0, 3}, {2, 0, 5},
        {4, 1, 3}, {2, 1, 4}, {5, 1, 2}, {3, 1, 5},
        {4, 3, 0}, {2, 4, 0}, {5, 2, 0}, {3, 5, 0},
        {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1}
    };
    vector<Algorithm> result(96);
    size_t length = this->twists.size();
    for (size_t i = 0; i < 96; ++i) {
        result[i].twists.resize(length);
    }
    for (size_t i = 0; i < 24; ++i) {
        const int* table = transform_table[i];
        for (size_t index = 0; index < length; ++index) {
            size_t inversed_index = length - 1 - index;
            int twist = this->twists[index];
            int result_twist = table[twist >> 3] << 2 ^ (twist & 7);
            result[i].twists[index] = result_twist;
            result[i + 24].twists[inversed_index] =
                this->inverse_twist_table[result_twist];
            result[i + 48].twists[index] = result_twist < 16
                ? this->inverse_twist_table[result_twist]
                : 40 - result_twist;
            result[i + 72].twists[inversed_index] =
                this->inverse_twist_table[result[i + 48].twists[index]];
        }
        result[i].normalize();
        result[i + 24].normalize();
        result[i + 48].normalize();
        result[i + 72].normalize();
    }
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    return result;
}
