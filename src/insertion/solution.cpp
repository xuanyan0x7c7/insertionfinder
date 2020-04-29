#include <cstddef>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/termcolor.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::MergedInsertion;
using InsertionFinder::Solution;


namespace {
    void print_solution_status(std::ostream& out, const Solution& solution) {
        out << termcolor::bold << "Total moves: " << termcolor::reset
        << termcolor::yellow << solution.final_solution.length() << termcolor::reset << ", "
        << termcolor::yellow << solution.cancellation << termcolor::reset
        << " move" << (solution.cancellation == 1 ? "" : "s") << " cancelled." << std::endl
        << termcolor::bold << "Final solution: " << termcolor::reset << solution.final_solution;
    }
};


std::ostream& operator<<(std::ostream& out, const Solution& solution) {
    for (size_t index = 0; index < solution.insertions.size(); ++index) {
        solution.insertions[index].print(out, index);
        out << std::endl;
    }
    print_solution_status(out, solution);
    return out;
}

void Solution::print(std::ostream& out, const std::vector<MergedInsertion>& merged_insertions) const {
    size_t initial_order = 0;
    for (const MergedInsertion& merged_insertion: merged_insertions) {
        merged_insertion.print(out, initial_order, *this);
        out << std::endl;
        initial_order += merged_insertion.insertions.size();
    }
    print_solution_status(out, *this);
}
