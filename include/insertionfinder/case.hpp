#pragma once
#include <cstdint>
#include <algorithm>
#include <exception>
#include <istream>
#include <ostream>
#include <utility>
#include <vector>
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
        Cube state;
        std::vector<InsertionAlgorithm> list;
    private:
        std::uint64_t mask;
        bool parity;
        int corner_cycles;
        int edge_cycles;
    public:
        Case() {}
        explicit Case(const Cube& state):
            state(state),
            mask(state.mask()),
            parity(state.has_parity()),
            corner_cycles(state.corner_cycles()),
            edge_cycles(state.edge_cycles()) {}
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        static int compare(const Case& lhs, const Case& rhs) noexcept;
        bool operator==(const Case& rhs) const noexcept {
            return this->state == rhs.state;
        }
        bool operator!=(const Case& rhs) const noexcept {
            return this->state != rhs.state;
        }
    public:
        const Cube& get_state() const noexcept {
            return this->state;
        }
        std::uint64_t get_mask() const noexcept {
            return this->mask;
        }
        bool has_parity() const noexcept {
            return this->parity;
        }
        bool get_parity() const noexcept {
            return this->parity || Cube::center_cycles[this->get_placement()] > 1;
        }
        int get_corner_cycles() const noexcept {
            return this->corner_cycles;
        }
        int get_edge_cycles() const noexcept {
            return this->edge_cycles;
        }
        Rotation get_placement() const noexcept {
            return this->state.placement();
        }
        const std::vector<InsertionAlgorithm>& algorithm_list() const noexcept {
            return this->list;
        }
    public:
        bool contains_algorithm(const Algorithm& algorithm) const noexcept {
            return std::find(this->list.cbegin(), this->list.cend(), algorithm) != this->list.cend();
        }
        template<class T> void add_algorithm(T&& algorithm) {
            if (!this->contains_algorithm(algorithm)) {
                this->list.emplace_back(std::forward<T>(algorithm));
            }
        }
        void merge_algorithms(Case&& from) {
            for (InsertionAlgorithm& algorithm: from.list) {
                this->add_algorithm(std::move(algorithm));
            }
        }
        void sort_algorithms() noexcept {
            std::sort(this->list.begin(), this->list.end());
        }
    };
};
