#include <cstddef>
#include <ostream>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Insertion;


std::ostream& operator<<(std::ostream& out, const Insertion& insertion) {
    insertion.print(out);
    return out;
}

void Insertion::print(std::ostream& out) const {
    const Algorithm& skeleton = this->skeleton;
    size_t insert_place = this->insert_place;
    if (insert_place > 0) {
        skeleton.print(out, 0, insert_place);
        out << ' ';
    }
    out << "[@]";
    if (insert_place < skeleton.length()) {
        out << ' ';
        skeleton.print(out, insert_place, skeleton.length());
    }
    out << std::endl;
    out << "Insert at @: " << *this->insertion;
}

void Insertion::print(std::ostream& out, size_t index) const {
    const Algorithm& skeleton = this->skeleton;
    size_t insert_place = this->insert_place;
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
    out << "Insert at @" << index + 1 << ": " << *this->insertion;
}
