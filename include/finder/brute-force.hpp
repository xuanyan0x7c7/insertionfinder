#pragma once
#include <array>
#include <mutex>
#include <vector>
#include <algorithm.hpp>
#include <case.hpp>
#include <cube.hpp>
#include <finder/finder.hpp>

namespace InsertionFinder {
    class BruteForceFinder: public Finder {
    private:
        class Worker {
        private:
            BruteForceFinder& finder;
            std::vector<Insertion> solving_step;
            std::size_t cancellation;
        public:
            explicit Worker(BruteForceFinder& finder);
        public:
            void search(
                const CycleStatus& cycle_status,
                std::size_t begin, std::size_t end
            );
        private:
            void search_last_parity(std::size_t begin, std::size_t end);
            void search_last_corner_cycle(std::size_t begin, std::size_t end);
            void search_last_edge_cycle(std::size_t begin, std::size_t end);
            void search_last_placement(
                int placement,
                std::size_t begin, std::size_t end
            );
            void try_insertion(
                std::size_t insert_place,
                const Cube& state,
                const CycleStatus& cycle_status,
                bool swapped = false
            );
            void try_last_insertion(
                std::size_t insert_place,
                int case_index,
                bool swapped = false
            ) {
                if (case_index == -1) {
                    return;
                }
                this->solution_found(
                    insert_place,
                    this->finder.cases[case_index]
                );
            }
            void solution_found(std::size_t insert_place, const Case& _case);
            void update_fewest_moves();
            bool continue_searching(const Algorithm& algorithm) noexcept {
                return algorithm.length() <= this->finder.fewest_moves;
            }
        };
    private:
        std::mutex fewest_moves_mutex;
    public:
        Finder::Status search_core(std::size_t max_threads) override;
        void run_worker(
            const CycleStatus& cycle_status,
            std::size_t begin, std::size_t end
        );
    };
};
