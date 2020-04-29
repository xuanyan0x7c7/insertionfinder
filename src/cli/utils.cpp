#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <univalue.h>
#include <insertionfinder/termcolor.hpp>
#include "utils.hpp"
using std::int64_t;
using std::size_t;
using Algorithm = InsertionFinder::Algorithm;
using Insertion = InsertionFinder::Insertion;
using Solution = InsertionFinder::Solution;
namespace Details = InsertionFinder::Details;


void Details::print_duration(std::ostream& out, int64_t duration) {
    out << termcolor::bold << "Time usage: " << termcolor::reset
        << std::fixed << std::setprecision(3) << termcolor::yellow;
    if (duration < 1000) {
        out << duration << termcolor::reset << " nanoseconds." << std::endl;
    } else if (duration < 1'000'000) {
        out << duration / 1e3 << termcolor::reset << " microseconds." << std::endl;
    } else if (duration < 1'000'000'000) {
        out << duration / 1e6 << termcolor::reset << " milliseconds." << std::endl;
    } else if (duration < 60 * INT64_C(1'000'000'000)) {
        out << duration / 1e9 << termcolor::reset << " seconds." << std::endl;
    } else if (duration < 60 * 60 * INT64_C(1'000'000'000)) {
        int64_t milliseconds = (duration + 500'000) / 1'000'000;
        out << milliseconds / (60 * 1000)
            << std::right << std::setfill('0')
            << ':' << std::setw(2) << milliseconds / 1000 % 60
            << '.'<< std::setw(3) << milliseconds % 1000
            << termcolor::reset << std::endl;
    } else {
        int64_t milliseconds = (duration + 500'000) / 1'000'000;
        out << milliseconds / (60 * 60 * 1000)
            << std::right << std::setfill('0')
            << ':' << std::setw(2) << milliseconds / (60 * 1000) % 60
            << ':' << std::setw(2) << milliseconds / 1000 % 60
            << ':' << std::setw(3) << milliseconds % 1000
            << termcolor::reset << '.' << std::endl;
    }
}


UniValue Details::create_json_solution(const Algorithm& skeleton, const Solution& solution) {
    UniValue insertion_list(UniValue::VARR);
    for (const Insertion& insertion: solution.insertions) {
        UniValue insertion_map(UniValue::VOBJ);
        insertion_map.pushKV("skeleton", insertion.skeleton.str());
        insertion_map.pushKV("insert_place", static_cast<int>(insertion.insert_place));
        insertion_map.pushKV("insertion", insertion.insertion->str());
        insertion_list.push_back(insertion_map);
    }
    UniValue merged_insertion_list(UniValue::VARR);
    size_t initial_order = 0;
    for (const auto& sub_solution: solution.merge_insertions(skeleton)) {
        UniValue insertion_list(UniValue::VARR);
        for (const auto& [place, insertions]: sub_solution.get_insertions()) {
            UniValue algorithms(UniValue::VARR);
            for (auto [insertion, order]: insertions) {
                UniValue algorithm_object(UniValue::VOBJ);
                algorithm_object.pushKV("algorithm", insertion->str());
                algorithm_object.pushKV("order", static_cast<int>(order));
                algorithms.push_back(algorithm_object);
            }
            UniValue place_object(UniValue::VOBJ);
            place_object.pushKV("insert_place", static_cast<int>(place));
            place_object.pushKV("algorithms", algorithms);
            insertion_list.push_back(place_object);
        }
        UniValue insertion_map(UniValue::VOBJ);
        insertion_map.pushKV("skeleton", sub_solution.skeleton.str());
        insertion_map.pushKV("insertions", insertion_list);
        merged_insertion_list.push_back(insertion_map);
        initial_order += sub_solution.insertions.size();
    }
    UniValue solution_map(UniValue::VOBJ);
    solution_map.pushKV("final_solution", solution.final_solution.str());
    solution_map.pushKV("cancellation", static_cast<int>(solution.cancellation));
    solution_map.pushKV("insertions", insertion_list);
    solution_map.pushKV("merged_insertions", merged_insertion_list);
    return solution_map;
}
