#include <cstddef>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/termcolor.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Solution;


std::ostream& operator<<(std::ostream& out, const Solution& solution) {
    for (size_t index = 0; index < solution.insertions.size(); ++index) {
        solution.insertions[index].print(out, index);
        out << std::endl;
    }
    out << termcolor::bold << "Total moves: " << termcolor::reset
        << termcolor::yellow << solution.final_solution.length() << termcolor::reset << ", "
        << termcolor::yellow << solution.cancellation << termcolor::reset
        << " move" << (solution.cancellation == 1 ? "" : "s") << " cancelled." << std::endl
        << termcolor::bold << "Final solution: " << termcolor::reset << solution.final_solution;
    return out;
}
