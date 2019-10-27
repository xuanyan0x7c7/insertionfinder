#include <cstdint>
#include <cstring>
#include <array>
#include <functional>
#include <istream>
#include <iterator>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <range/v3/all.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/twist.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint32_t;
using std::uint8_t;
using InsertionFinder::Algorithm;
using InsertionFinder::AlgorithmError;
using InsertionFinder::AlgorithmStreamError;
using InsertionFinder::Cube;
using InsertionFinder::Rotation;
using InsertionFinder::Twist;
namespace Details = InsertionFinder::Details;


namespace {
    struct Pattern {
        const char* pattern_string;
        Rotation rotation;
    };

    constexpr Pattern pattern_table[9] = {
        {"x", 12}, {"x2", 8}, {"x'", 4},
        {"y", 3}, {"y2", 2}, {"y'", 1},
        {"z", 20}, {"z2", 10}, {"z'", 16}
    };
};


Algorithm::Algorithm(const char* algorithm_string) {
    static const std::regex twists_regex(
        R"(\s*([UDRLFBxyz][2']?)\s*)",
        std::regex_constants::ECMAScript | std::regex_constants::optimize
    );
    Rotation rotation = 0;
    std::cmatch match_result;
    const char* string_view = algorithm_string;
    while (std::regex_search(string_view, match_result, twists_regex)) {
        if (match_result.position()) {
            throw AlgorithmError(algorithm_string);
        }
        const std::string match_string = match_result.str(1);
        if (
            auto pattern = ranges::find_if(pattern_table, [&](const Pattern& pattern) {
                return pattern.pattern_string == match_string;
            });
            pattern == pattern_table + std::size(pattern_table)
        ) {
            this->twists.push_back(Twist(match_string) * rotation);
        } else {
            rotation = pattern->rotation * rotation;
        }
        string_view += match_result.length();
    }
    if (*string_view) {
        throw AlgorithmError(algorithm_string);
    }
    this->rotation = rotation.inverse();
}


int Algorithm::compare(const Algorithm& lhs, const Algorithm& rhs) noexcept {
    const auto& t1 = lhs.twists;
    const auto& t2 = rhs.twists;
    if (int x = static_cast<int>(t1.size()) - static_cast<int>(t2.size())) {
        return x;
    }
    if (int x = std::memcmp(t1.data(), t2.data(), t1.size() * sizeof(Twist))) {
        return x;
    }
    return lhs.rotation - rhs.rotation;
}


std::ostream& operator<<(std::ostream& out, const Algorithm& algorithm) {
    algorithm.print(out, 0, algorithm.twists.size());
    if (algorithm.rotation && !algorithm.twists.empty()) {
        out << ' ' << algorithm.rotation;
    }
    return out;
}

void Algorithm::print(std::ostream& out, size_t begin, size_t end) const {
    if (begin >= end) {
        return;
    }
    out << this->twists[begin];
    for (Twist twist: ranges::views::slice(this->twists, begin + 1, ranges::end)) {
        out << ' ' << twist;
    }
}

std::string Algorithm::str() const {
    std::stringstream stream;
    stream << *this;
    return stream.str();
}


void Algorithm::save_to(std::ostream& out) const {
    uint8_t length = this->twists.size();
    out.write(reinterpret_cast<char*>(&length), 1);
    auto data = std::make_unique<char[]>(length);
    for (size_t i = 0; i < length; ++i) {
        data[i] = this->twists[i];
    }
    out.write(data.get(), length);
    char rotation_data = this->rotation;
    out.write(&rotation_data, 1);
}

void Algorithm::read_from(std::istream& in) {
    uint8_t length;
    in.read(reinterpret_cast<char*>(&length), 1);
    if (in.gcount() != 1) {
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

    this->begin_mask = Details::twist_mask(this->twists[0].inverse());
    if (length > 1 && this->twists[0] >> 3 == this->twists[1] >> 3) {
        this->begin_mask |= Details::twist_mask(this->twists[1].inverse());
    }
    this->end_mask = Details::twist_mask((this->twists[length - 1] * this->rotation).inverse());
    if (length > 1 && this->twists[length - 1] >> 3 == this->twists[length - 2] >> 3) {
        this->end_mask |= Details::twist_mask((this->twists[length - 2] * this->rotation).inverse());
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
    for (Twist twist: rhs.twists) {
        result.twists.push_back(twist * this->rotation.inverse());
    }
    result.rotation = this->rotation * rhs.rotation;
    return result;
}

Algorithm& Algorithm::operator+=(const Algorithm& rhs) {
    this->twists.reserve(this->twists.size() + rhs.twists.size());
    for (Twist twist: rhs.twists) {
        this->twists.push_back(twist * this->rotation.inverse());
    }
    this->rotation *= rhs.rotation;
    return *this;
}


size_t std::hash<Algorithm>::operator()(const Algorithm& algorithm) const noexcept {
    size_t result = algorithm.rotation;
    for (Twist twist: algorithm.twists) {
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

void Algorithm::rotate(Rotation rotation) {
    for (Twist& twist: this->twists) {
        twist *= rotation;
    }
    this->rotation = rotation.inverse() * this->rotation * rotation;
}

void Algorithm::inverse() noexcept {
    ranges::reverse(this->twists);
    this->rotate(this->rotation);
    this->rotation = this->rotation.inverse();
}

Algorithm Algorithm::inverse(const Algorithm& algorithm) {
    Algorithm result;
    result.twists.reserve(algorithm.twists.size());
    for (Twist twist: ranges::views::reverse(algorithm.twists)) {
        result.twists.push_back(twist * algorithm.rotation);
    }
    result.rotation = algorithm.rotation.inverse();
    return result;
}
