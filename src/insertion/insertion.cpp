#include <cstddef>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/termcolor.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Insertion;


std::ostream& operator<<(std::ostream& out, const Insertion& insertion) {
    insertion.print(out);
    return out;
}

void Insertion::print(std::ostream& out) const {
    Algorithm result = this->skeleton.insert(*this->insertion, this->insert_place).first;
    size_t insert_place = this->insert_place;
    if (insert_place > 0) {
        this->skeleton.print(out, 0, insert_place);
        out << ' ';
    }
    out << termcolor::bold << "[@]" << termcolor::reset;
    if (insert_place < this->skeleton.length()) {
        out << ' ';
        this->skeleton.print(out, insert_place, this->skeleton.length());
    }
    out << std::endl
        << termcolor::bold << "Insert at @: " << termcolor::reset << *this->insertion
        << termcolor::dark << termcolor::italic << " (+" << this->insertion->length() << " -"
        << this->skeleton.length() + this->insertion->length() - result.length() << ')' << termcolor::reset;
}

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
