#include <array>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


namespace {
    constexpr const char* twist_string[24] = {
        "", "U", "U2", "U'",
        "", "D", "D2", "D'",
        "", "R", "R2", "R'",
        "", "L", "L2", "L'",
        "", "F", "F2", "F'",
        "", "B", "B2", "B'"
    };

    struct Transform {
        int transform[3];
        int additional_twist;
    };

    const unordered_map<string, Transform> pattern_table({
        {"x", {{4, 2, 1}, -1}},
        {"[r]", {{4, 2, 1}, -1}},
        {"[l']", {{4, 2, 1}, -1}},
        {"x2", {{1, 2, 5}, -1}},
        {"[r2]", {{1, 2, 5}, -1}},
        {"[l2]", {{1, 2, 5}, -1}},
        {"x'", {{5, 2, 0}, -1}},
        {"[r']", {{5, 2, 0}, -1}},
        {"[l]", {{5, 2, 0}, -1}},
        {"y", {{0, 5, 2}, -1}},
        {"[u]", {{0, 5, 2}, -1}},
        {"[d']", {{0, 5, 2}, -1}},
        {"y2", {{0, 3, 5}, -1}},
        {"[u2]", {{0, 3, 5}, -1}},
        {"[d2]", {{0, 3, 5}, -1}},
        {"y'", {{0, 4, 3}, -1}},
        {"[u']", {{0, 4, 3}, -1}},
        {"[d]", {{0, 4, 3}, -1}},
        {"z", {{3, 0, 4}, -1}},
        {"[f]", {{3, 0, 4}, -1}},
        {"[b']", {{3, 0, 4}, -1}},
        {"z2", {{1, 3, 4}, -1}},
        {"[f2]", {{1, 3, 4}, -1}},
        {"[b2]", {{1, 3, 4}, -1}},
        {"z'", {{2, 1, 4}, -1}},
        {"[f']", {{2, 1, 4}, -1}},
        {"[b]", {{2, 1, 4}, -1}},
        {"Uw", {{0, 5, 2}, 5}},
        {"2Uw", {{0, 5, 2}, 5}},
        {"Uw2", {{0, 3, 5}, 6}},
        {"2Uw2", {{0, 3, 5}, 6}},
        {"Uw'", {{0, 4, 3}, 7}},
        {"2Uw'", {{0, 4, 3}, 7}},
        {"Dw", {{0, 4, 3}, 1}},
        {"2Dw", {{0, 4, 3}, 1}},
        {"Dw2", {{0, 3, 5}, 2}},
        {"2Dw2", {{0, 3, 5}, 2}},
        {"Dw'", {{0, 5, 2}, 3}},
        {"2Dw'", {{0, 5, 2}, 3}},
        {"Rw", {{4, 2, 1}, 13}},
        {"2Rw", {{4, 2, 1}, 13}},
        {"Rw2", {{1, 2, 5}, 14}},
        {"2Rw2", {{1, 2, 5}, 14}},
        {"Rw'", {{5, 2, 0}, 15}},
        {"2Rw'", {{5, 2, 0}, 15}},
        {"Lw", {{5, 2, 0}, 9}},
        {"2Lw", {{5, 2, 0}, 9}},
        {"Lw2", {{1, 2, 5}, 10}},
        {"2Lw2", {{1, 2, 5}, 10}},
        {"Lw'", {{4, 2, 1}, 11}},
        {"2Lw'", {{4, 2, 1}, 11}},
        {"Fw", {{3, 0, 4}, 21}},
        {"2Fw", {{3, 0, 4}, 21}},
        {"Fw2", {{1, 3, 4}, 22}},
        {"2Fw2", {{1, 3, 4}, 22}},
        {"Fw'", {{2, 1, 4}, 23}},
        {"2Fw'", {{2, 1, 4}, 23}},
        {"Bw", {{2, 1, 4}, 17}},
        {"2Bw", {{2, 1, 4}, 17}},
        {"Bw2", {{1, 3, 4}, 18}},
        {"2Bw2", {{1, 3, 4}, 18}},
        {"Bw'", {{3, 0, 4}, 19}},
        {"2Bw'", {{3, 0, 4}, 19}}
    });
};


Algorithm::Algorithm(const string& algorithm_string) {
    static const regex twists_regex(
        R"(\s*((?:2?[UDRLFB]w|[UDRLFB])[2']?|[xyz][2']?|\[[udrlfb][2']?\])\s*)",
        regex_constants::ECMAScript | regex_constants::optimize
    );
    array<int, 3> transform = {0, 2, 4};
    smatch match_result;
    string temp_string = algorithm_string;
    while (regex_search(temp_string, match_result, twists_regex)) {
        if (match_result.position()) {
            throw AlgorithmError(algorithm_string);
        }
        if (
            auto find_result = pattern_table.find(match_result[1]);
            find_result != pattern_table.cend()
        ) {
            const auto& [pattern_transform, twist] = find_result->second;
            if (twist != -1) {
                this->twists.push_back(transform_twist(transform.data(), twist));
            }
            array<int, 3> new_transform;
            for (int i = 0; i < 3; ++i) {
                int temp = pattern_transform[i];
                new_transform[i] = transform[temp >> 1] ^ (temp & 1);
            }
            transform = new_transform;
        } else {
            int twist = find(
                twist_string, twist_string + 24,
                string(match_result[1])
            ) - twist_string;
            this->twists.push_back(transform_twist(transform.data(), twist));
        }
        temp_string = match_result.suffix();
    }
    if (!temp_string.empty()) {
        throw AlgorithmError(algorithm_string);
    }
    this->rotation = 0;

    this->cancel_moves();
}


int Algorithm::compare(const Algorithm& lhs, const Algorithm& rhs) noexcept {
    const auto& t1 = lhs.twists;
    const auto& t2 = rhs.twists;
    if (int x = static_cast<int>(t1.size()) - static_cast<int>(t2.size())) {
        return x;
    }
    for (size_t i = 0, l = t1.size(); i < l; ++i) {
        if (t1[i] != t2[i]) {
            return t1[i] - t2[i];
        }
    }
    if (lhs.rotation != rhs.rotation) {
        return lhs.rotation - rhs.rotation;
    }
    return 0;
}


ostream& operator<<(ostream& out, const Algorithm& algorithm) {
    static const char* rotation_string[24] = {
        "", "y", "y2", "y'",
        "x", "x y", "x y2", "x y'",
        "x2", "x2 y", "z2", "x2 y'",
        "x'", "x' y", "x' y2", "x' y'",
        "z", "z y", "z y2", "z y'",
        "z'", "z' y", "z' y2", "z' y'"
    };
    algorithm.print(out, 0, algorithm.twists.size());
    if (algorithm.rotation && !algorithm.twists.empty()) {
        out << ' ';
    }
    out << rotation_string[algorithm.rotation];
    return out;
}

void Algorithm::print(ostream& out, size_t begin, size_t end) const {
    if (begin >= end) {
        return;
    }
    out << twist_string[this->twists[begin]];
    for (size_t i = begin; ++i < end;) {
        out << ' ' << twist_string[this->twists[i]];
    }
}

string Algorithm::str() const {
    stringstream stream;
    stream << *this;
    return stream.str();
}


void Algorithm::save_to(ostream& out) const {
    size_t length = this->twists.size();
    out.write(reinterpret_cast<char*>(&length), sizeof(size_t));
    auto data = make_unique<char[]>(length);
    for (size_t i = 0; i < length; ++i) {
        data[i] = this->twists[i];
    }
    out.write(data.get(), length);
    char rotation_data = this->rotation;
    out.write(&rotation_data, 1);
}

void Algorithm::read_from(istream& in) {
    size_t length;
    in.read(reinterpret_cast<char*>(&length), sizeof(size_t));
    if (in.gcount() != sizeof(size_t)) {
        throw AlgorithmStreamError();
    }
    auto data = make_unique<char[]>(length);
    in.read(data.get(), length);
    if (static_cast<size_t>(in.gcount()) != length) {
        throw AlgorithmStreamError();
    }
    this->twists = vector<int>(data.get(), data.get() + length);
    char rotation_data;
    in.read(&rotation_data, 1);
    if (static_cast<size_t>(in.gcount()) != 1) {
        throw AlgorithmStreamError();
    }
    this->rotation = rotation_data;

    const int* transform = rotation_permutation[this->rotation];
    auto& twists = this->twists;
    this->begin_mask = twist_mask(Algorithm::inverse_twist[twists[0]]);
    if (length > 1 && twists[0] >> 3 == twists[1] >> 3) {
        this->begin_mask |= twist_mask(Algorithm::inverse_twist[twists[1]]);
    }
    this->end_mask = twist_mask(Algorithm::inverse_twist[
        transform_twist(transform, twists[length - 1])
    ]);
    if (length > 1 && twists[length - 1] >> 3 == twists[length - 2] >> 3) {
        this->end_mask |= twist_mask(Algorithm::inverse_twist[
            transform_twist(transform, twists[length - 2])
        ]);
    }
    if (length > 2 && this->begin_mask & this->end_mask) {
        this->set_up_mask = 0;
        uint32_t set_up_mask = (this->begin_mask & this->end_mask) >> 24;
        for (size_t i = 0; i < 6; ++i) {
            if (set_up_mask & 1 << i) {
                this->set_up_mask |= 0xe << (i << 2) | 1 << (i + 24);
            }
        }
        this->set_up_mask &= this->end_mask;
    } else {
        this->set_up_mask = 0;
    }
}


size_t hash<Algorithm>::operator()(const Algorithm& algorithm) const noexcept {
    size_t result = algorithm.rotation;
    for (int twist: algorithm.twists) {
        result = result * 31 + twist;
    }
    return result;
}


void Algorithm::detect_rotation() noexcept {
    Cube cube;
    cube.twist(*this);
    this->rotation = cube.best_placement().placement();
}
