#include <cstddef>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Solution;


std::ostream& operator<<(std::ostream& out, const Solution& solution) {
    for (size_t index = 0; index < solution.insertions.size(); ++index) {
        solution.insertions[index].print(out, index);
        out << std::endl;
    }
    out << "Total moves: " << solution.final_solution.length() << ", "
        << solution.cancellation << " move" << (solution.cancellation == 1 ? "" : "s")
        << " cancelled." << std::endl
        << "Final solution: " << solution.final_solution;
    return out;
}
