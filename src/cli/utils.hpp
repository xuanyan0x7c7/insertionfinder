#pragma once
#include <cstdint>
#include <ostream>
#include <insertionfinder/insertion.hpp>


namespace InsertionFinder::Details {
    void print_insertions(std::ostream& out, const InsertionFinder::Solution& solution);
    void print_solution_status(std::ostream& out, const InsertionFinder::Solution& solution);
    void print_duration(std::ostream& out, std::int64_t duration);
};
