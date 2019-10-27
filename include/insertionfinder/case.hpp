#pragma once
#include <cstdint>
#include <exception>
#include <istream>
#include <ostream>
#include <utility>
#include <vector>
#include <range/v3/all.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/twist.hpp>

namespace InsertionFinder {
    struct CaseStreamError: std::exception {
        const char* what() const noexcept override {
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
        Case() {}
        explicit Case(const Cube& state);
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        static int compare(const Case& lhs, const Case& rhs) noexcept;
        bool operator==(const Case& rhs) const noexcept {
            return this->_state == rhs._state;
        }
        bool operator!=(const Case& rhs) const noexcept {
            return this->_state != rhs._state;
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
        bool parity() const noexcept {
            return this->_has_parity || Cube::center_cycles[this->_state.placement()] > 1;
        }
        int corner_cycles() const noexcept {
            return this->_corner_cycles;
        }
        int edge_cycles() const noexcept {
            return this->_edge_cycles;
        }
        Rotation rotation() const noexcept {
            return this->_state.placement();
        }
        const std::vector<Algorithm>& algorithm_list() const noexcept {
            return this->list;
        }
    public:
        bool contains_algorithm(const Algorithm& algorithm) const {
            return ranges::find(this->list, algorithm) != this->list.cend();
        }
        template<class T> void add_algorithm(T&& algorithm) {
            if (!this->contains_algorithm(algorithm)) {
                this->list.emplace_back(std::forward<T>(algorithm));
            }
        }
        void merge_algorithms(Case&& from) {
            for (Algorithm& algorithm: from.list) {
                this->add_algorithm(std::move(algorithm));
            }
        }
        void sort_algorithms() {
            ranges::sort(this->list);
        }
    };
};
