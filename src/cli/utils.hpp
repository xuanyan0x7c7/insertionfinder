#pragma once
#include <cstdint>
#include <ostream>
#include <univalue.h>
#include <insertionfinder/insertion.hpp>

namespace InsertionFinder::Details {
    void print_duration(std::ostream& out, std::int64_t duration);
    UniValue
    create_json_solution(const InsertionFinder::Algorithm& skeleton, const InsertionFinder::Solution& solution);
};
