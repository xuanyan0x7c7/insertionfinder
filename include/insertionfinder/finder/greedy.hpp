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
        struct CheapInsertion {
            const Algorithm* skeleton;
            std::size_t insert_place;
            const Algorithm* insertion;
            bool swapped;
        };
        struct SolvingStep {
            CheapInsertion insertion;
            CycleStatus cycle_status;
        };
        struct PartialState {
            std::atomic<std::size_t> fewest_moves;
            std::mutex fewest_moves_mutex;
        };
        struct Skeleton {
            const Algorithm* skeleton;
            const CycleStatus* cycle_status;
        };
        class Worker {
        private:
            GreedyFinder& finder;
            const Algorithm& skeleton;
            const CycleStatus& cycle_status;
        public:
            explicit Worker(
                GreedyFinder& finder,
                const Algorithm& skeleton,
                const CycleStatus& cycle_status
            ): finder(finder), skeleton(skeleton), cycle_status(cycle_status) {}
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
        std::vector<std::unordered_map<Algorithm, SolvingStep>> partial_solutions;
        std::unordered_map<Algorithm, std::size_t> cycles_mapping;
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
