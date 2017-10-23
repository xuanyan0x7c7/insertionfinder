#pragma once
#include <cstdint>
#include <algorithm>
#include <exception>
#include <istream>
#include <ostream>
#include <vector>
#include <algorithm.hpp>
#include <cube.hpp>

namespace InsertionFinder {
    struct CaseStreamError: std::exception {
        virtual const char* what() const noexcept override {
            return "Failed to read case from stream";
        }
    };

    class Case {
    private:
        Cube state;
        std::vector<Algorithm> algorithm_list;
    private:
        std::uint32_t mask;
        bool has_parity;
        int corner_cycles;
        int edge_cycles;
    public:
        Case(const Cube& state);
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    private:
        bool contains_algorithm(const Algorithm& algorithm) const {
            return std::find(
                this->algorithm_list.cbegin(), this->algorithm_list.cend(),
                algorithm
            ) != this->algorithm_list.cend();
        }
    public:
        void add_algorithm(const Algorithm& algorithm) {
            if (!this->contains_algorithm(algorithm)) {
                this->algorithm_list.push_back(algorithm);
            }
        }
        void sort_algorithms() {
            std::sort(this->algorithm_list.begin(), this->algorithm_list.end());
        }
    };
};
