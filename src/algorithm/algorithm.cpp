#include <cstdint>
#include <cstring>
#include <array>
#include <functional>
#include <istream>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <range/v3/all.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint32_t;
using std::uint_fast8_t;
using InsertionFinder::Algorithm;
using InsertionFinder::AlgorithmError;
using InsertionFinder::AlgorithmStreamError;
using InsertionFinder::Cube;
namespace Details = InsertionFinder::Details;


namespace {
    uint_fast8_t parse_twist(const std::string& twist_string) {
        uint_fast8_t offset = 0;
        switch (twist_string[0]) {
        case 'U':
            offset = 0;
            break;
        case 'D':
            offset = 4;
            break;
        case 'R':
            offset = 8;
            break;
        case 'L':
            offset = 12;
            break;
        case 'F':
            offset = 16;
            break;
        case 'B':
            offset = 20;
            break;
        }
        if (twist_string.length() == 1) {
            return offset + 1;
        } else if (twist_string[1] == '2') {
            return offset + 2;
        } else {
            return offset + 3;
        }
    }

    struct Pattern {
        const char* pattern_string;
        uint_fast8_t transform_table[3];
    };

    constexpr const std::array<Pattern, 9> pattern_table = {{
        {"x", {4, 2, 1}},
        {"x2", {1, 2, 5}},
        {"x'", {5, 2, 0}},
        {"y", {0, 5, 2}},
        {"y2", {0, 3, 5}},
        {"y'", {0, 4, 3}},
        {"z", {3, 0, 4}},
        {"z2", {1, 3, 4}},
        {"z'", {2, 1, 4}}
    }};
};


Algorithm::Algorithm(const char* algorithm_string) {
    static const std::regex twists_regex(
        R"(\s*([UDRLFBxyz][2']?)\s*)",
        std::regex_constants::ECMAScript | std::regex_constants::optimize
    );
    uint_fast8_t transform[3] = {0, 2, 4};
    std::cmatch match_result;
    const char* temp_string = algorithm_string;
    while (std::regex_search(temp_string, match_result, twists_regex)) {
        if (match_result.position()) {
            throw AlgorithmError(algorithm_string);
        }
        const std::string match_string = match_result.str(1);
        if (
            auto pattern = ranges::find_if(pattern_table, [&](const Pattern& pattern) {
                return pattern.pattern_string == match_string;
            });
            pattern == pattern_table.cend()
        ) {
            this->twists.push_back(Details::transform_twist(transform, parse_twist(match_string)));
        } else {
            uint_fast8_t new_transform[3];
            for (size_t i = 0; i < 3; ++i) {
                uint_fast8_t temp = pattern->transform_table[i];
                new_transform[i] = transform[temp >> 1] ^ (temp & 1);
            }
            std::memcpy(transform, new_transform, 3 * sizeof(uint_fast8_t));
        }
        temp_string += match_result.length();
    }
    if (*temp_string) {
        throw AlgorithmError(algorithm_string);
    }
    for (unsigned i = 0; i < 24; ++i) {
        const uint_fast8_t* permutation = Details::rotation_permutation[Cube::inverse_center[i]];
        if (permutation[0] == transform[0] && permutation[1] == transform[1]) {
            this->rotation = i;
            break;
        }
    }
}


int Algorithm::compare(const Algorithm& lhs, const Algorithm& rhs) noexcept {
    const auto& t1 = lhs.twists;
    const auto& t2 = rhs.twists;
    if (int x = static_cast<int>(t1.size()) - static_cast<int>(t2.size())) {
        return x;
    }
    if (int x = std::memcmp(t1.data(), t2.data(), t1.size() * sizeof(uint_fast8_t))) {
        return x;
    }
    return lhs.rotation - rhs.rotation;
}


std::ostream& operator<<(std::ostream& out, const Algorithm& algorithm) {
    static constexpr const char* rotation_string[24] = {
        "", "y", "y2", "y'",
        "x", "x y", "x y2", "x y'",
        "x2", "x2 y", "z2", "x2 y'",
        "x'", "x' y", "x' y2", "x' y'",
        "z", "z y", "z y2", "z y'",
        "z'", "z' y", "z' y2", "z' y'"
    };
    algorithm.print(out, 0, algorithm.twists.size());
    if (algorithm.rotation && !algorithm.twists.empty()) {
        out << ' ' << rotation_string[algorithm.rotation];
    }
    return out;
}

void Algorithm::print(std::ostream& out, size_t begin, size_t end) const {
    static constexpr const char* twist_string[24] = {
        "", "U", "U2", "U'",
        "", "D", "D2", "D'",
        "", "R", "R2", "R'",
        "", "L", "L2", "L'",
        "", "F", "F2", "F'",
        "", "B", "B2", "B'"
    };
    if (begin >= end) {
        return;
    }
    auto iter = this->twists.cbegin() + begin;
    out << twist_string[*iter];
    for (auto iter_end = this->twists.cbegin() + end; ++iter < iter_end;) {
        out << ' ' << twist_string[*iter];
    }
}

std::string Algorithm::str() const {
    std::stringstream stream;
    stream << *this;
    return stream.str();
}


void Algorithm::save_to(std::ostream& out) const {
    size_t length = this->twists.size();
    out.write(reinterpret_cast<char*>(&length), sizeof(size_t));
    auto data = std::make_unique<char[]>(length);
    for (size_t i = 0; i < length; ++i) {
        data[i] = this->twists[i];
    }
    out.write(data.get(), length);
    char rotation_data = this->rotation;
    out.write(&rotation_data, 1);
}

void Algorithm::read_from(std::istream& in) {
    size_t length;
    in.read(reinterpret_cast<char*>(&length), sizeof(size_t));
    if (in.gcount() != sizeof(size_t)) {
        throw AlgorithmStreamError();
    }
    auto data = std::make_unique<char[]>(length);
    in.read(data.get(), length);
    if (in.gcount() != length) {
        throw AlgorithmStreamError();
    }
    this->twists.assign(data.get(), data.get() + length);
    char rotation_data;
    in.read(&rotation_data, 1);
    if (in.gcount() != 1) {
        throw AlgorithmStreamError();
    }
    this->rotation = rotation_data;

    const uint_fast8_t* transform = Details::rotation_permutation[this->rotation];
    const auto& twists = this->twists;
    this->begin_mask = Details::twist_mask(Algorithm::inverse_twist[twists[0]]);
    if (length > 1 && twists[0] >> 3 == twists[1] >> 3) {
        this->begin_mask |= Details::twist_mask(Algorithm::inverse_twist[twists[1]]);
    }
    this->end_mask = Details::twist_mask(
        Algorithm::inverse_twist[Details::transform_twist(transform, twists[length - 1])]
    );
    if (length > 1 && twists[length - 1] >> 3 == twists[length - 2] >> 3) {
        this->end_mask |= Details::twist_mask(
            Algorithm::inverse_twist[Details::transform_twist(transform, twists[length - 2])]
        );
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


Algorithm Algorithm::operator+(const Algorithm& rhs) const {
    Algorithm result;
    result.twists.reserve(this->twists.size() + rhs.twists.size());
    result.twists.assign(this->twists.cbegin(), this->twists.cend());
    ranges::move(
        rhs.twists | ranges::views::transform(
            std::bind(
                Details::transform_twist,
                Details::rotation_permutation[Cube::inverse_center[this->rotation]],
                std::placeholders::_1
            )
        ),
        ranges::back_inserter(result.twists)
    );
    result.rotation = Cube::placement_twist(this->rotation, rhs.rotation);
    return result;
}

Algorithm& Algorithm::operator+=(const Algorithm& rhs) {
    this->twists.reserve(this->twists.size() + rhs.twists.size());
    ranges::move(
        rhs.twists | ranges::views::transform(
            std::bind(
                Details::transform_twist,
                Details::rotation_permutation[Cube::inverse_center[this->rotation]],
                std::placeholders::_1
            )
        ),
        ranges::back_inserter(this->twists)
    );
    this->rotation = Cube::placement_twist(this->rotation, rhs.rotation);
    return *this;
}


size_t std::hash<Algorithm>::operator()(const Algorithm& algorithm) const noexcept {
    size_t result = algorithm.rotation;
    for (uint_fast8_t twist: algorithm.twists) {
        result = result * 31 + twist;
    }
    return result;
}


void Algorithm::detect_rotation() noexcept {
    Cube cube = Cube() * *this;
    this->rotation = cube.best_placement().placement();
}


void Algorithm::normalize() noexcept {
    for (size_t i = 1, length = this->twists.size(); i < length; ++i) {
        if (this->swappable(i) && this->twists[i - 1] > this->twists[i]) {
            this->swap_adjacent(i++);
        }
    }
}

void Algorithm::rotate(int rotation) {
    const uint_fast8_t* table = Details::rotation_permutation[rotation];
    for (uint_fast8_t& twist: this->twists) {
        twist = Details::transform_twist(table, twist);
    }
}

void Algorithm::inverse() {
    ranges::reverse(this->twists);
    this->rotate(this->rotation);
    this->rotation = Cube::inverse_center[this->rotation];
}

Algorithm Algorithm::inverse(const Algorithm& algorithm) {
    Algorithm result;
    result.twists.reserve(algorithm.twists.size());
    ranges::move(
        algorithm.twists
            | ranges::views::reverse
            | ranges::views::transform(
                std::bind(
                    Details::transform_twist,
                    Details::rotation_permutation[algorithm.rotation],
                    std::placeholders::_1
                )
            ),
        ranges::back_inserter(result.twists)
    );
    result.rotation = Cube::inverse_center[algorithm.rotation];
    return result;
}
