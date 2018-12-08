#pragma once
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>
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
            const Algorithm& skeleton;
            const CycleStatus& cycle_status;
            const std::size_t cancellation;
        public:
            explicit Worker(
                GreedyFinder& finder,
                const Algorithm& skeleton,
                const CycleStatus& cycle_status,
                std::size_t cancellation
            ):
                finder(finder), skeleton(skeleton),
                cycle_status(cycle_status), cancellation(cancellation) {}
        public:
            void search();
        private:
            void search_last_parity();
            void search_last_corner_cycle();
            void search_last_edge_cycle();
            void search_last_placement(int placement);
            void try_insertion(
                std::size_t insert_place,
                const Cube& state,
                bool swapped = false
            );
            void try_last_insertion(
                std::size_t insert_place,
                int case_index,
                bool swapped = false
            );
            void solution_found(
                std::size_t insert_place,
                bool swapped,
                const Case& _case
            );
        };
    private:
        const std::size_t threshold;
        std::vector<std::vector<std::pair<Algorithm, SolvingStep>>>
        partial_solution_list;
        std::unordered_map<Algorithm, SolvingStep> partial_solution_map;
        PartialState* partial_states;
    public:
        GreedyFinder(
            const Algorithm& scramble, const Algorithm& skeleton,
            const std::vector<Case>& cases,
            std::size_t threshold = 2
        ):
            Finder(scramble, skeleton, cases),
            threshold(threshold),
            partial_states(nullptr) {}
        ~GreedyFinder() {
            delete[] partial_states;
        }
    protected:
        void search_core(
            const CycleStatus& cycle_status,
            const SearchParams& params
        ) override;
    private:
        void run_worker(
            const std::vector<Skeleton>& skeletons,
            std::size_t start, std::size_t step
        );
    };
};
