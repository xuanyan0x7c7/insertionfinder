#include <array>
#include <algorithm>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <formula.hpp>
#include <iostream>
using namespace std;
using namespace InsertionFinder;


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
        {"Uw2", {{0, 3, 5}, 6}},
        {"Uw'", {{0, 4, 3}, 7}},
        {"Dw", {{0, 4, 3}, 1}},
        {"Dw2", {{0, 3, 5}, 2}},
        {"Dw'", {{0, 5, 2}, 3}},
        {"Rw", {{4, 2, 1}, 13}},
        {"Rw2", {{1, 2, 5}, 14}},
        {"Rw'", {{5, 2, 0}, 15}},
        {"Lw", {{5, 2, 0}, 9}},
        {"Lw2", {{1, 2, 5}, 10}},
        {"Lw'", {{4, 2, 1}, 11}},
        {"Fw", {{3, 0, 4}, 21}},
        {"Fw2", {{1, 3, 4}, 22}},
        {"Fw'", {{2, 1, 4}, 23}},
        {"Bw", {{2, 1, 4}, 17}},
        {"Bw2", {{1, 3, 4}, 18}},
        {"Bw'", {{3, 0, 4}, 19}}
    });

    int transform_twist(const array<int, 3>& transform, int twist) {
        return transform[twist >> 3] << 2 ^ (twist & 7);
    }
};


Formula::Formula(const string& formula_string) {
    static const regex twists_regex(
        R"(\s*((?:[UDRLFB]|2?[UDRLFB]w)[2']?|[xyz][2']?|\[[udrlfb][2']?\])\s*)",
        regex_constants::ECMAScript | regex_constants::optimize
    );
    array<int, 3> transform = {0, 2, 4};
    smatch match_result;
    string temp_string = formula_string;
    while (regex_search(temp_string, match_result, twists_regex)) {
        if (match_result.position()) {
            throw FormulaError(formula_string);
        }
        if (
            auto find_result = pattern_table.find(match_result[1]);
            find_result != pattern_table.cend()
        ) {
            const auto& [pattern_transform, twist] = find_result->second;
            if (twist != -1) {
                this->twists.push_back(transform_twist(transform, twist));
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
            this->twists.push_back(transform_twist(transform, twist));
        }
        temp_string = match_result.suffix();
    }
    if (!temp_string.empty()) {
        throw FormulaError(formula_string);
    }
}


ostream& operator<<(ostream& out, const Formula& formula) {
    formula.print(out, 0, formula.twists.size());
    return out;
}

void Formula::print(ostream& out, size_t begin, size_t end) const {
    if (begin >= end) {
        return;
    }
    out << twist_string[this->twists[begin]];
    for (size_t i = begin; ++i < end;) {
        out << ' ' << twist_string[this->twists[i]];
    }
}

string Formula::to_string() const {
    stringstream stream;
    stream << *this;
    return stream.str();
}
