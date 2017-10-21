#pragma once
#include <array>
#include <cstdint>
#include <exception>
#include <limits>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace InsertionFinder {class Formula;};
std::ostream& operator<<(std::ostream&, const InsertionFinder::Formula&);

namespace InsertionFinder {
    class FormulaError: std::exception {
    private:
        const std::string formula_string;
        const std::string explanation_string;
    public:
        FormulaError(std::string formula_string):
            formula_string(std::move(formula_string)),
            explanation_string("Invalid formula string: " + this->formula_string)
            {}
    public:
        virtual const char* what() const noexcept override {
            return explanation_string.c_str();
        }
    };

    class Formula {
    public:
        static constexpr std::array<int, 24> inverse_twist_table = {
            0, 3, 2, 1,
            4, 7, 6, 5,
            8, 11, 10, 9,
            12, 15, 14, 13,
            16, 19, 18, 17,
            20, 23, 22, 21
        };
    private:
        std::vector<int> twists;
    private:
        std::uint32_t begin_mask;
        std::uint32_t end_mask;
        std::uint32_t set_up_mask;
    public:
        Formula() {}
        Formula(const Formula&) = default;
        Formula(Formula&&) = default;
        Formula& operator=(const Formula&) = default;
        Formula& operator=(Formula&&) = default;
        ~Formula() = default;
        explicit Formula(const std::string& formula_string);
    public:
        std::size_t length() const noexcept {
            return this->twists.size();
        }
        int operator[](std::size_t index) const {
            return this->twists[index];
        }
    public:
        static int compare(const Formula& lhs, const Formula& rhs) noexcept;
        bool operator==(const Formula& rhs) const noexcept {
            return this->compare(*this, rhs) == 0;
        }
        bool operator<(const Formula& rhs) const noexcept {
            return this->compare(*this, rhs) < 0;
        }
    public:
        friend std::ostream& ::operator<<(std::ostream& out, const Formula& formula);
        void print(std::ostream& out, std::size_t begin, std::size_t end) const;
        std::string to_string() const;
    private:
        std::size_t cancel_moves();
    public:
        std::pair<std::uint32_t, std::uint32_t>
        get_insert_place_mask(std::size_t insert_place) const;
        std::pair<Formula, std::size_t>
        insert(const Formula& insertion, std::size_t insert_place) const;
        bool is_worthy_insertion(
            const Formula& insertion, std::size_t insert_place,
            const std::pair<std::uint32_t, std::uint32_t>& insert_place_mask,
            std::size_t fewest_twists = std::numeric_limits<std::size_t>::max()
        ) const;
    public:
        bool swappable(std::size_t place) const {
            return place > 0 && place < this->twists.size()
                && this->twists[place - 1] >> 3 == this->twists[place] >> 3;
        }
        void swap_adjacent(std::size_t place) {
            std::swap(this->twists[place - 1], this->twists[place]);
        }
    public:
        void normalize() noexcept;
        std::vector<Formula> generate_isomorphisms() const;
    };
};
