#pragma once
#include <atomic>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/asio.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>

namespace InsertionFinder {
    class GreedyFinder: public Finder {
    private:
        struct SolvingStep {
            const Algorithm* skeleton;
            std::size_t insert_place;
            const Algorithm* insertion;
            bool swapped;
            CycleStatus cycle_status;
            std::size_t cancellation;
        };
        struct PartialState {
            std::atomic<std::size_t> fewest_moves;
            std::mutex fewest_moves_mutex;
        };
        struct Skeleton {
            const Algorithm* skeleton;
            const CycleStatus* cycle_status;
            std::size_t cancellation;
        };
        class Worker {
        private:
            GreedyFinder& finder;
            boost::asio::thread_pool& pool;
            const Algorithm& skeleton;
            const CycleStatus& cycle_status;
            const std::size_t cancellation;
        public:
            explicit Worker(
                GreedyFinder& finder,
                boost::asio::thread_pool& pool,
                const Algorithm& skeleton,
                const CycleStatus& cycle_status,
                std::size_t cancellation
            ):
                finder(finder), pool(pool), skeleton(skeleton),
                cycle_status(cycle_status), cancellation(cancellation) {}
        public:
            void search();
        private:
            void search_last_corner_cycle();
            void search_last_edge_cycle();
            void search_last_placement(int placement);
            void try_insertion(std::size_t insert_place, const Cube& state, bool swapped = false);
            void try_last_insertion(std::size_t insert_place, int case_index, bool swapped = false);
            void solution_found(std::size_t insert_place, bool swapped, const Case& _case);
        };
    public:
        struct Options {
            bool enable_replacement;
            std::size_t greedy_threshold;
            std::size_t replacement_threshold;
        };
    private:
        const Options options;
        std::vector<std::vector<std::pair<Algorithm, SolvingStep>>> partial_solution_list;
        std::unordered_map<Algorithm, SolvingStep> partial_solution_map;
        std::deque<PartialState> partial_states;
        std::mutex worker_mutex;
    public:
        GreedyFinder(
            const Algorithm& scramble, const std::vector<Algorithm>& skeletons,
            const std::vector<Case>& cases, Options options
        ): Finder(scramble, skeletons, cases), options(options), partial_solution_list(1), partial_states(1) {}
        GreedyFinder(
            const Algorithm& scramble, std::vector<Algorithm>&& skeleton,
            const std::vector<Case>& cases, Options options
        ):
            Finder(scramble, std::move(skeleton), cases),
            options(options), partial_solution_list(1), partial_states(1) {}
        GreedyFinder(
            const Algorithm& scramble, const Algorithm& skeleton,
            const std::vector<Case>& cases, Options options
        ): Finder(scramble, skeleton, cases), options(options), partial_solution_list(1), partial_states(1) {}
    protected:
        void search_core(const SearchParams& params) override;
    private:
        void run_worker(boost::asio::thread_pool& pool, Algorithm&& skeleton, const SolvingStep& step);
    };
};
