#pragma once
#include <cstddef>
#include <mutex>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/asio.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/twist.hpp>
#include <insertionfinder/improver/improver.hpp>

namespace InsertionFinder {
    class SliceImprover: public Improver {
    private:
        struct SolvingStep {
            const Algorithm* skeleton;
            std::size_t insert_place;
            const Algorithm* insertion;
            bool swapped;
            Rotation placement;
            std::size_t cancellation;
        };
        class Worker {
        private:
            SliceImprover& improver;
            boost::asio::thread_pool& pool;
            const Algorithm& skeleton;
            Rotation placement;
            const std::size_t cancellation;
        public:
            explicit Worker(
                SliceImprover& improver,
                boost::asio::thread_pool& pool,
                const Algorithm& skeleton,
                Rotation placement,
                std::size_t cancellation
            ): improver(improver), pool(pool), skeleton(skeleton), placement(placement), cancellation(cancellation) {}
        public:
            void search();
        private:
            void try_insertion(std::size_t insert_place, const Cube& state, bool swapped = false);
        };
    public:
        struct Options {
            std::size_t threshold;
        };
    private:
        const Options options;
        std::mutex fewest_moves_mutex;
        std::unordered_set<Algorithm> partial_solution_list;
        std::unordered_map<Algorithm, SolvingStep> partial_solution_map;
        std::mutex worker_mutex;
    public:
        template<class Skeleton> SliceImprover(Skeleton&& skeleton, const std::vector<Case>& cases, Options options):
            Improver(std::forward<Skeleton>(skeleton), cases), options(options) {}
    protected:
        void search_core(const SearchParams& params) override;
    private:
        void run_worker(boost::asio::thread_pool& pool, Algorithm&& skeleton, const SolvingStep& step);
    };
};
