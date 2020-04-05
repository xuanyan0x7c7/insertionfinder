#pragma once
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <insertionfinder/insertion.hpp>


namespace InsertionFinder::Details {
    void print_insertion(std::ostream& out, const InsertionFinder::Insertion& insertion, std::size_t index);
    void print_duration(std::ostream& out, std::int64_t duration);
};
