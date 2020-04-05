#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
#include "utils.hpp"
using std::size_t;
using std::int64_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Insertion;
namespace Details = InsertionFinder::Details;


void Details::print_insertion(std::ostream& out, const Insertion& insertion, size_t index) {
    const Algorithm& skeleton = insertion.skeleton;
    size_t insert_place = insertion.insert_place;
    if (insert_place > 0) {
        skeleton.print(out, 0, insert_place);
        out << ' ';
    }
    out << "[@" << index + 1 << ']';
    if (insert_place < skeleton.length()) {
        out << ' ';
        skeleton.print(out, insert_place, skeleton.length());
    }
    out << std::endl;
    out << "Insert at @" << index + 1 << ": " << *insertion.insertion << std::endl;
}

void Details::print_duration(std::ostream& out, int64_t duration) {
    out << "Time usage: " << std::fixed << std::setprecision(3);
    if (duration < 1000) {
        out << duration << " nanoseconds." << std::endl;
    } else if (duration < 1'000'000) {
        out << duration / 1e3 << " microseconds." << std::endl;
    } else if (duration < 1'000'000'000) {
        out << duration / 1e6 << " milliseconds." << std::endl;
    } else if (duration < 60 * INT64_C(1'000'000'000)) {
        out << duration / 1e9 << " seconds." << std::endl;
    } else if (duration < 60 * 60 * INT64_C(1'000'000'000)) {
        int64_t milliseconds = (duration + 500'000) / 1'000'000;
        out << milliseconds / (60 * 1000)
            << std::right << std::setfill('0')
            << ':' << std::setw(2) << milliseconds / 1000 % 60
            << '.'<< std::setw(3) << milliseconds % 1000 << std::endl;
    } else {
        int64_t milliseconds = (duration + 500'000) / 1'000'000;
        out << milliseconds / (60 * 60 * 1000)
            << std::right << std::setfill('0')
            << ':' << std::setw(2) << milliseconds / (60 * 1000) % 60
            << ':' << std::setw(2) << milliseconds / 1000 % 60
            << ':' << std::setw(3) << milliseconds % 1000 << '.' << std::endl;
    }
}
