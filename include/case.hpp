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
        Cube _state;
        std::vector<Algorithm> list;
    private:
        std::uint32_t _mask;
        bool _has_parity;
        int _corner_cycles;
        int _edge_cycles;
    public:
        Case(const Cube& state);
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        const Cube& state() const {
            return this->_state;
        }
        std::uint32_t mask() const {
            return this->_mask;
        }
        bool has_parity() const {
            return this->_has_parity;
        }
        int corner_cycles() const {
            return this->_corner_cycles;
        }
        int edge_cycles() const {
            return this->_edge_cycles;
        }
        const std::vector<Algorithm>& algorithm_list() const {
            return this->list;
        }
    private:
        bool contains_algorithm(const Algorithm& algorithm) const {
            return std::find(
                this->list.cbegin(), this->list.cend(),
                algorithm
            ) != this->list.cend();
        }
    public:
        void add_algorithm(const Algorithm& algorithm) {
            if (!this->contains_algorithm(algorithm)) {
                this->list.push_back(algorithm);
            }
        }
        void sort_algorithms() {
            std::sort(this->list.begin(), this->list.end());
        }
    };
};
