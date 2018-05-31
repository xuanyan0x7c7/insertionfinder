#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <algorithm.hpp>
#include <case.hpp>
#include <cube.hpp>
#include <finder/finder.hpp>

namespace InsertionFinder {
    class GreedyFinder: public Finder {
    private:
        struct CheapInsertion {
            std::shared_ptr<Algorithm> skeleton;
            std::size_t insert_place;
            const Algorithm* insertion;
        };
        struct SolvingStep {
            std::vector<CheapInsertion> steps;
            CycleStatus cycle_status;
        };
        struct PartialState {
            std::atomic<std::size_t> fewest_moves;
            std::mutex fewest_moves_mutex;
        };
        class Worker {
        private:
            GreedyFinder& finder;
            const SolvingStep& solving_step;
        public:
            explicit Worker(
                GreedyFinder& finder,
                const SolvingStep& solving_step
            ): finder(finder), solving_step(solving_step) {}
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
                std::shared_ptr<Algorithm> skeleton,
                std::size_t insert_place,
                const Case& _case
            );
        };
    private:
        const std::size_t threshold;
        std::vector<std::vector<SolvingStep>> partial_solutions;
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
            std::size_t max_threads
        ) override;
    private:
        void run_worker(std::size_t start, std::size_t step);
    };
};
