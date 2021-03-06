#include <cstddef>
#include <algorithm>
#include <deque>
#include <limits>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/termcolor.hpp>
#include <insertionfinder/twist.hpp>
using std::size_t;
using Algorithm = InsertionFinder::Algorithm;
using Insertion = InsertionFinder::Insertion;
using MergedInsertion = InsertionFinder::MergedInsertion;
using Rotation = InsertionFinder::Rotation;
using Solution = InsertionFinder::Solution;


namespace {
    constexpr size_t invalid = std::numeric_limits<size_t>::max() >> 1;

    class MultiInsertion {
    private:
        Algorithm skeleton;
        Algorithm current_skeleton;
        std::vector<std::vector<std::size_t>> insert_places_before;
        std::vector<std::vector<std::size_t>> insert_places_after;
        std::vector<const Algorithm*> insertions;
        std::vector<std::size_t> positions;
    public:
        MultiInsertion(const Algorithm& skeleton);
    public:
        const Algorithm& get_skeleton() const noexcept {
            return this->skeleton;
        }
        const Algorithm& get_current_skeleton() const noexcept {
            return this->current_skeleton;
        }
        std::vector<std::pair<std::size_t, std::vector<std::size_t>>> get_insert_places() const;
        const std::vector<const Algorithm*>& get_insertions() const noexcept {
            return this->insertions;
        }
    public:
        bool try_insert(const Insertion& insertion);
    };
};


MultiInsertion::MultiInsertion(const Algorithm& skeleton):
    skeleton(skeleton), current_skeleton(skeleton),
    insert_places_before(skeleton.length() + 1),
    insert_places_after(skeleton.length() + 1) {
    this->positions.reserve(skeleton.length());
    for (size_t i = 0; i < skeleton.length(); ++i) {
        this->positions.push_back(i);
    }
}

std::vector<std::pair<size_t, std::vector<size_t>>> MultiInsertion::get_insert_places() const {
    std::vector<std::pair<size_t, std::vector<size_t>>> result;
    for (size_t i = 0; i <= this->skeleton.length(); ++i) {
        const auto& before = this->insert_places_before[i];
        const auto& after = this->insert_places_after[i];
        if (!before.empty() || !after.empty()) {
            result.emplace_back(i, std::vector<size_t>(before.crbegin(), before.crend()));
            result.back().second.insert(result.back().second.cend(), after.cbegin(), after.cend());
        }
    }
    return result;
}

bool MultiInsertion::try_insert(const Insertion& insertion) {
    size_t length = this->current_skeleton.length();
    if (insertion.skeleton.length() != length) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        if (this->current_skeleton[i] == insertion.skeleton[i]) {
        } else if (
            this->current_skeleton.swappable(i + 1)
            && this->current_skeleton[i] == insertion.skeleton[i + 1]
            && this->current_skeleton[i + 1] == insertion.skeleton[i]
        ) {
            if (insertion.insert_place == i + 1) {
                if (this->positions[i + 1] - this->positions[i] == 1) {
                    this->skeleton.swap_adjacent(this->positions[i] + 1);
                    this->current_skeleton.swap_adjacent(i + 1);
                } else {
                    return false;
                }
            }
            ++i;
        } else {
            return false;
        }
    }
    if (insertion.insert_place == 0) {
        this->insert_places_before.front().push_back(this->insertions.size());
    } else if (insertion.insert_place == length) {
        this->insert_places_after.back().push_back(this->insertions.size());
    } else if (this->positions[insertion.insert_place] != invalid) {
        this->insert_places_after[this->positions[insertion.insert_place]].push_back(this->insertions.size());
    } else if (this->positions[insertion.insert_place - 1] != invalid) {
        this->insert_places_before[this->positions[insertion.insert_place - 1] + 1].push_back(this->insertions.size());
    } else {
        return false;
    }
    this->insertions.push_back(insertion.insertion);
    auto [
        insertion_result, skeleton_marks, insertion_marks
    ] = this->current_skeleton.insert_return_marks(*insertion.insertion, insertion.insert_place);
    for (size_t i = 1; i < insertion.insert_place; ++i) {
        if (skeleton_marks[i - 1] != 0 && skeleton_marks[i] == 0 && this->positions[i] - this->positions[i - 1] == 1) {
            this->skeleton.swap_adjacent(this->positions[i]);
            this->current_skeleton.swap_adjacent(i);
            std::swap(skeleton_marks[i - 1], skeleton_marks[i]);
            if (skeleton_marks[i - 1] == 1) {
                insertion_result.swap_adjacent(i);
            }
            break;
        }
    }
    for (size_t i = insertion.insert_place + 1; i < length; ++i) {
        if (skeleton_marks[i - 1] == 0 && skeleton_marks[i] != 0 && this->positions[i] - this->positions[i - 1] == 1) {
            this->skeleton.swap_adjacent(this->positions[i]);
            this->current_skeleton.swap_adjacent(i);
            std::swap(skeleton_marks[i - 1], skeleton_marks[i]);
            if (skeleton_marks[i] == 1) {
                insertion_result.swap_adjacent(i);
            }
            break;
        }
    }
    std::vector<std::size_t> new_positions(insertion_result.length(), invalid);
    for (size_t i = 0; i < std::min(insertion.insert_place, insertion_result.length()); ++i) {
        if (skeleton_marks[i] == 0) {
            new_positions[i] = this->positions[i];
        }
    }
    for (size_t i = insertion.insert_place; i < length; ++i) {
        if (skeleton_marks[i] == 0) {
            new_positions[insertion_result.length() + i - length] = this->positions[i];
        }
    }
    this->current_skeleton = std::move(insertion_result);
    this->positions = std::move(new_positions);
    return true;
}


std::vector<std::pair<size_t, std::vector<MergedInsertion::SubInsertion>>> MergedInsertion::get_insertions() const {
    std::vector<std::pair<size_t, std::vector<SubInsertion>>> result;
    result.reserve(this->insert_places.size());
    for (const auto& [insert_place, orders]: this->insert_places) {
        std::vector<SubInsertion> insertions;
        insertions.reserve(orders.size());
        for (size_t order: orders) {
            insertions.push_back(SubInsertion{&this->insertions[order], order});
        }
        result.emplace_back(insert_place, std::move(insertions));
    }
    return result;
}


void MergedInsertion::print(std::ostream& out, std::size_t initial_order, const Solution& solution) const {
    auto [
        result, skeleton_marks, insertion_masks
    ] = this->skeleton.multi_insert_return_marks(this->insertions, this->insert_places);
    bool skeleton_has_space = this->insert_places.front().first > 0;
    this->skeleton.print(out, skeleton_marks, 0, this->insert_places.front().first);
    for (size_t order: this->insert_places.front().second) {
        if (skeleton_has_space) {
            out << ' ';
        }
        skeleton_has_space = true;
        out << termcolor::bold << "[@" << initial_order + order + 1 << ']' << termcolor::reset;
    }
    for (size_t i = 1; i < this->insert_places.size(); ++i) {
        out << ' ';
        this->skeleton.print(out, skeleton_marks, this->insert_places[i - 1].first, this->insert_places[i].first);
        for (size_t order: this->insert_places[i].second) {
            out << termcolor::bold << " [@" << initial_order + order + 1 << ']' << termcolor::reset;
        }
    }
    if (this->insert_places.back().first < this->skeleton.length()) {
        out << ' ';
        this->skeleton.print(out, skeleton_marks, this->insert_places.back().first, this->skeleton.length());
    }
    for (const auto& [_, orders]: this->insert_places) {
        for (size_t order: orders) {
            const Algorithm& insertion = this->insertions[order];
            out << std::endl
                << termcolor::bold << "Insert at @" << initial_order + order + 1 << ": " << termcolor::reset;
            insertion.print(out, insertion_masks[order]);
            size_t length_before = solution.insertions[initial_order + order].skeleton.length();
            size_t length_after = initial_order + order + 1 >= solution.insertions.size()
                ? solution.final_solution.length()
                : solution.insertions[initial_order + order + 1].skeleton.length();
            out << termcolor::dark << termcolor::italic << " (+" << insertion.length()
                << " -" << length_before + insertion.length() - length_after << ')' << termcolor::reset;
        }
    }
}


std::vector<MergedInsertion> Solution::merge_insertions(const Algorithm& skeleton) const {
    std::vector<MultiInsertion> multi_insertion_list;
    multi_insertion_list.emplace_back(skeleton);
    auto multi_insertion = multi_insertion_list.rbegin();
    for (const Insertion& insertion: this->insertions) {
        if (bool success = multi_insertion->try_insert(insertion); !success) {
            multi_insertion_list.emplace_back(multi_insertion->get_current_skeleton());
            multi_insertion = multi_insertion_list.rbegin();
            multi_insertion->try_insert(insertion);
        }
    }
    std::vector<MergedInsertion> merged_insertion_list;
    merged_insertion_list.reserve(multi_insertion_list.size());
    for (const MultiInsertion& multi_insertion: multi_insertion_list) {
        MergedInsertion merged_insertion;
        merged_insertion.skeleton = multi_insertion.get_skeleton();
        merged_insertion.final_solution = multi_insertion.get_current_skeleton();
        merged_insertion.insert_places = multi_insertion.get_insert_places();
        auto insertions = multi_insertion.get_insertions();
        size_t insertion_count = insertions.size();
        merged_insertion.insertions.reserve(insertion_count);
        for (size_t i = insertion_count; i-- > 0;) {
            Algorithm insertion = *insertions[i];
            Rotation rotation = 0;
            for (const auto& [_, orders]: merged_insertion.insert_places) {
                bool found = false;
                for (size_t order: orders) {
                    if (order == i) {
                        found = true;
                        break;
                    } else if (order < i) {
                        rotation *= insertions[order]->cube_rotation();
                    }
                }
                if (found) {
                    break;
                }
            }
            insertion.rotate(rotation);
            insertion.normalize();
            merged_insertion.insertions.emplace_back(std::move(insertion));
        }
        std::reverse(merged_insertion.insertions.begin(), merged_insertion.insertions.end());
        merged_insertion_list.emplace_back(std::move(merged_insertion));
    }
    return merged_insertion_list;
}
