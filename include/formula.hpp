#pragma once
#include <cstdint>
#include <exception>
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
    private:
        std::vector<int> twists;
        std::uint32_t begin_mask;
        std::uint32_t end_mask;
        std::uint32_t set_up_mask;
    public:
        explicit Formula(const std::string& formula_string);
    public:
        std::size_t length() const noexcept {
            return this->twists.size();
        }
        int operator[](std::size_t index) const {
            return this->twists[index];
        }
    public:
        friend std::ostream& ::operator<<(std::ostream& out, const Formula& formula);
        void print(std::ostream& out, std::size_t begin, std::size_t end) const;
        std::string to_string() const;
    };
};
