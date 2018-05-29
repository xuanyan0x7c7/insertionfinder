#pragma once
#include <memory>
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
        std::vector<std::vector<SolvingStep>> partial_solutions;
    public:
        using Finder::Finder;
        Finder::Status search_core(std::size_t max_threads) override;
    };
};
