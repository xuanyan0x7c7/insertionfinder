#include <cstddef>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/termcolor.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::MergedInsertion;
using InsertionFinder::Solution;


void Solution::print(std::ostream& out, const Algorithm& skeleton, bool expand) const {
    if (expand) {
        for (size_t index = 0; index < this->insertions.size(); ++index) {
            this->insertions[index].print(out, index);
            out << std::endl;
        }
    } else {
        size_t start_index = 0;
        for (const MergedInsertion& solution: this->merge_insertions(skeleton)) {
            solution.print(out, start_index, *this);
            out << std::endl;
            start_index += solution.insertions.size();
        }
    }
    out << termcolor::bold << "Total moves: " << termcolor::reset
        << termcolor::yellow << this->final_solution.length() << termcolor::reset << ", "
        << termcolor::yellow << this->cancellation << termcolor::reset
        << " move" << (this->cancellation == 1 ? "" : "s") << " cancelled." << std::endl
        << termcolor::bold << "Final solution: " << termcolor::reset << this->final_solution;
}
