#pragma once
#include <mutex>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>

namespace InsertionFinder {
    class BruteForceFinder: public Finder {
    private:
        class Worker {
        private:
            BruteForceFinder& finder;
            std::vector<Insertion> solving_step;
        public:
            Worker(BruteForceFinder& finder, const Algorithm& skeleton):
                finder(finder), solving_step({Insertion(skeleton)}) {}
        public:
            void search(const CycleStatus& cycle_status, std::size_t begin, std::size_t end);
        private:
            void search_last_corner_cycle(std::size_t begin, std::size_t end);
            void search_last_edge_cycle(std::size_t begin, std::size_t end);
            void search_last_placement(int placement, std::size_t begin, std::size_t end);
            void try_insertion(
                std::size_t insert_place,
                const Cube& state,
                CycleStatus cycle_status,
                bool swapped = false
            );
            void try_last_insertion(std::size_t insert_place, int case_index, bool swapped = false);
            void solution_found(std::size_t insert_place, const Case& _case);
            void update_fewest_moves();
        };
    private:
        std::mutex fewest_moves_mutex;
    public:
        using Finder::Finder;
    protected:
        void search_core(const SearchParams& params) override;
    private:
        void run_worker(const Algorithm& skeleton, CycleStatus cycle_status, std::size_t begin, std::size_t end) {
            Worker(*this, skeleton).search(cycle_status, begin, end);
        }
    };
};
