#pragma once
#include <atomic>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/asio.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
#include <insertionfinder/utils.hpp>

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
    public:
        struct Options {
            bool enable_replacement;
            std::size_t greedy_threshold;
            std::size_t replacement_threshold;
        };
    private:
        const Options options;
        std::vector<std::vector<std::pair<Algorithm, SolvingStep>>>
        partial_solution_list;
        std::unordered_map<Algorithm, SolvingStep> partial_solution_map;
        PartialState* partial_states;
        std::mutex worker_mutex;
    public:
        GreedyFinder(
            const Algorithm& scramble, const Algorithm& skeleton,
            const std::vector<Case>& cases, Options options
        ):
            Finder(scramble, skeleton, cases),
            options(options),
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
        template<class T> void run_worker(
            boost::asio::thread_pool& pool,
            T&& skeleton,
            const SolvingStep& step
        );
    };

    template <class T> void GreedyFinder::run_worker(
        boost::asio::thread_pool& pool,
        T&& skeleton,
        const SolvingStep& step
    ) {
        static_assert(std::is_same_v<
            Insertionfinder::Details::remove_cvref_t<T>,
            Algorithm
        >);
        std::lock_guard<std::mutex> lock(this->worker_mutex);
        auto [iter, inserted] = this->partial_solution_map.try_emplace(
            std::forward<T>(skeleton), step
        );
        const auto& old_skeleton = iter->first;
        auto& old_step = iter->second;
        if (inserted) {
            boost::asio::post(pool, [&]() {
                Worker(
                    *this, pool,
                    old_skeleton, old_step.cycle_status, old_step.cancellation
                ).search();
            });
        } else if (step.cancellation < old_step.cancellation) {
            old_step = step;
        }
    }
};
