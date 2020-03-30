#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
#include <atomic>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/twist.hpp>
#include <insertionfinder/utils.hpp>

namespace InsertionFinder {
    namespace FinderStatus {
        constexpr std::byte success {0};
        constexpr std::byte parity_algorithms_needed {1};
        constexpr std::byte corner_cycle_algorithms_needed {2};
        constexpr std::byte edge_cycle_algorithms_needed {4};
        constexpr std::byte center_algorithms_needed {8};
        constexpr std::byte full = parity_algorithms_needed
            | corner_cycle_algorithms_needed | edge_cycle_algorithms_needed
            | center_algorithms_needed;
    };

    class Finder {
    protected:
        struct CycleStatus {
            bool parity;
            std::int8_t corner_cycles;
            std::int8_t edge_cycles;
            std::int8_t placement;
            CycleStatus() = default;
            CycleStatus(bool parity, int corner_cycles, int edge_cycles, Rotation placement):
                parity(parity), corner_cycles(corner_cycles), edge_cycles(edge_cycles), placement(placement) {}
        };
    public:
        struct Result {
            std::byte status = FinderStatus::full;
            std::int64_t duration;
        };
        struct SearchParams {
            std::size_t search_target;
            double parity_multiplier;
            std::size_t max_threads;
        };
    protected:
        const Algorithm scramble;
        std::unordered_map<Algorithm, std::size_t> skeletons;
        const std::vector<Case>& cases;
        std::atomic<std::size_t> fewest_moves = std::numeric_limits<std::size_t>::max();
        std::vector<Solution> solutions;
        Result result;
        int parity_multiplier = 3;
        bool verbose = false;
    protected:
        int corner_cycle_index[6 * 24 * 24];
        int edge_cycle_index[10 * 24 * 24];
        int center_index[24];
        bool change_parity = false;
        bool change_corner = false;
        bool change_edge = false;
        bool change_center = false;
        const Cube scramble_cube;
        const Cube inverse_scramble_cube;
    public:
        template<class Scramble, class Skeleton, std::enable_if_t<std::is_convertible_v<Skeleton, Algorithm>, int> = 0>
        Finder(Scramble&& scramble, Skeleton&& skeleton, const std::vector<Case>& cases);
        template<class Scramble, class Range, std::enable_if_t<Details::is_iterable_v<Range>, int> = 0>
        Finder(Scramble&& scramble, Range&& range, const std::vector<Case>& cases);
        virtual ~Finder() = default;
    public:
        void search(const SearchParams& params);
    protected:
        void init();
        virtual void search_core(const SearchParams& params) = 0;
    public:
        std::size_t get_fewest_moves() const noexcept {
            return this->fewest_moves;
        }
        const std::vector<Solution>& get_solutions() const noexcept {
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
        int get_total_cycles(bool parity, int corner_cycles, int edge_cycles, Rotation placement) {
            int center_cycles = Cube::center_cycles[placement];
            return (center_cycles > 1 ? 0 : parity * this->parity_multiplier)
                + (corner_cycles + edge_cycles + center_cycles) * 2;
        }
    };

    template<class Scramble, class Skeleton, std::enable_if_t<std::is_convertible_v<Skeleton, Algorithm>, int>>
    Finder::Finder(Scramble&& scramble, Skeleton&& skeleton, const std::vector<Case>& cases):
        scramble(std::forward<Scramble>(scramble)), cases(cases),
        scramble_cube(Cube() * scramble), inverse_scramble_cube(Cube::inverse(this->scramble_cube)) {
        Algorithm algorithm(std::forward<Skeleton>(skeleton));
        algorithm.simplify();
        algorithm.normalize();
        this->skeletons.emplace(std::move(algorithm), 0);
        this->init();
    }

    template<class Scramble, class Range, std::enable_if_t<Details::is_iterable_v<Range>, int>>
    Finder::Finder(Scramble&& scramble, Range&& range, const std::vector<Case>& cases):
        scramble(std::forward<Scramble>(scramble)), cases(cases),
        scramble_cube(Cube() * scramble), inverse_scramble_cube(Cube::inverse(this->scramble_cube)) {
        for (auto&& x: std::forward<Range>(range)) {
            auto [skeleton, cancellation] = Details::Mapping<Algorithm, std::size_t>(std::forward<decltype(x)>(x));
            skeleton.simplify();
            skeleton.normalize();
            if (auto [node, inserted] = this->skeletons.try_emplace(std::move(skeleton), cancellation); !inserted) {
                if (node->second > cancellation) {
                    node->second = cancellation;
                }
            }
        }
        this->init();
    }
};
