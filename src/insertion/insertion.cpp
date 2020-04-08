#include <cstddef>
#include <ostream>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/termcolor.hpp>
using std::size_t;
using InsertionFinder::Insertion;


void Insertion::print(std::ostream& out, size_t index) const {
    auto [
        result, skeleton_marks, insertion_masks
    ] = this->skeleton.insert_return_marks(*this->insertion, this->insert_place);
    size_t insert_place = this->insert_place;
    if (insert_place > 0) {
        this->skeleton.print(out, skeleton_marks, 0, insert_place);
        out << ' ';
    }
    out << termcolor::bold << "[@" << index + 1 << ']' << termcolor::reset;
    if (insert_place < this->skeleton.length()) {
        out << ' ';
        this->skeleton.print(out, skeleton_marks, insert_place, this->skeleton.length());
    }
    out << std::endl << termcolor::bold << "Insert at @" << index + 1 << ": " << termcolor::reset;
    this->insertion->print(out, insertion_masks);
    out << termcolor::dark << termcolor::italic << " (+" << this->insertion->length() << " -"
        << this->skeleton.length() + this->insertion->length() - result.length() << ')' << termcolor::reset;
}
