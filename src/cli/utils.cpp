#include <cstdint>
#include <iomanip>
#include <ostream>
#include "utils.hpp"
using std::int64_t;
namespace Details = InsertionFinder::Details;


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
