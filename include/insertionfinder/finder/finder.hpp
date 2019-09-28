#pragma once
#include <cstdint>
#include <array>
#include <atomic>
#include <type_traits>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/utils.hpp>

namespace InsertionFinder {
    class Finder {
    protected:
        struct CycleStatus {
            bool parity;
            int corner_cycles;
            int edge_cycles;
            int placement;
        };
        int parity_multiplier = 3;
    public:
        struct Insertion {
            Algorithm skeleton;
            std::size_t insert_place;
            const Algorithm* insertion;
            explicit Insertion(
                const Algorithm& skeleton,
                std::size_t insert_place = 0,
                const Algorithm* insertion = nullptr
            ):
                skeleton(skeleton),
                insert_place(insert_place),
                insertion(insertion)
                {}
            explicit Insertion(
                Algorithm&& skeleton,
                std::size_t insert_place = 0,
                const Algorithm* insertion = nullptr
            ):
                skeleton(std::move(skeleton)),
                insert_place(insert_place),
                insertion(insertion)
                {}
        };
        struct Solution {
            std::vector<Insertion> insertions;
            std::size_t cancellation;
            Solution(const std::vector<Insertion>& insertions):
                insertions(insertions), cancellation(0) {}
            Solution(std::vector<Insertion>&& insertions):
                insertions(std::move(insertions)), cancellation(0) {}
        };
        enum class Status {
            SUCCESS,
            SUCCESS_SOLVED,
            FAILURE_PARITY_ALGORITHMS_NEEDED,
            FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED,
            FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED,
            FAILURE_CENTER_ALGORITHMS_NEEDED
        };
        struct Result {
            Status status;
            std::int64_t duration;
        };
        struct SearchParams {
            std::size_t search_target;
            double parity_multiplier;
            std::size_t max_threads;
        };
    protected:
        const Algorithm scramble;
        const Algorithm skeleton;
        const std::vector<Case>& cases;
        std::atomic<std::size_t> fewest_moves;
        std::vector<Solution> solutions;
        Result result;
        bool verbose;
    protected:
        std::array<int, 6 * 24 * 24> corner_cycle_index;
        std::array<int, 10 * 24 * 24> edge_cycle_index;
        std::array<int, 24> center_index;
        bool change_parity;
        bool change_corner;
        bool change_edge;
        bool change_center;
        Cube scramble_cube;
        Cube inverse_scramble_cube;
    public:
        Finder(
            const Algorithm& scramble, const Algorithm& skeleton,
            const std::vector<Case>& cases
        );
        virtual ~Finder() {}
    public:
        void search(const SearchParams& params);
    protected:
        virtual void search_core(
            const CycleStatus& cycle_status,
            const SearchParams& params
        ) = 0;
    public:
        std::size_t get_fewest_moves() const noexcept {
            return this->fewest_moves;
        }
        const std::vector<Solution> get_solutions() const noexcept {
            return this->solutions;
        }
        Result get_result() const noexcept {
            return this->result;
        }
    public:
        void set_verbose(bool verbose = true) noexcept {
            this->verbose = verbose;
        }
    public:
        inline int get_total_cycles(
            bool parity, int corner_cycles, int edge_cycles, int placement
        ) {
            int center_cycles = Cube::center_cycles[placement];
            return (center_cycles > 1 ? 0 : parity * this->parity_multiplier)
                + (corner_cycles + edge_cycles + center_cycles) * 2;
        }
    };
};
