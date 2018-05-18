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
        explicit Case(const Cube& state);
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        static int compare(const Case& lhs, const Case& rhs) noexcept;
        bool operator==(const Case& rhs) const noexcept {
            return this->compare(*this, rhs) == 0;
        }
        bool operator<(const Case& rhs) const noexcept {
            return this->compare(*this, rhs) < 0;
        }
    public:
        const Cube& state() const noexcept {
            return this->_state;
        }
        std::uint32_t mask() const noexcept {
            return this->_mask;
        }
        bool has_parity() const noexcept {
            return this->_has_parity;
        }
        int corner_cycles() const noexcept {
            return this->_corner_cycles;
        }
        int edge_cycles() const noexcept {
            return this->_edge_cycles;
        }
        int rotation() const noexcept {
            return this->_state.placement();
        }
        const std::vector<Algorithm>& algorithm_list() const noexcept {
            return this->list;
        }
    public:
        bool contains_algorithm(const Algorithm& algorithm) const {
            return std::find(
                this->list.cbegin(), this->list.cend(),
                algorithm
            ) != this->list.cend();
        }
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
