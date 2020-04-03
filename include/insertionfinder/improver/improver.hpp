#pragma once
#include <cstddef>
#include <atomic>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/utils.hpp>

namespace InsertionFinder {
    class Improver {
    public:
        struct Result {
            std::int64_t duration;
        };
        struct SearchParams {
            std::size_t max_threads;
        };
    protected:
        Algorithm skeleton;
        const std::vector<Case>& cases;
        std::atomic<std::size_t> fewest_moves;
        std::vector<Solution> solutions;
        Result result;
    protected:
        Cube skeleton_cube;
        Cube inverse_skeleton_cube;
    public:
        template<class Skeleton> Improver(Skeleton&& skeleton, const std::vector<Case>& cases);
        virtual ~Improver() = default;
    public:
        void search(const SearchParams& params);
    protected:
        void init() {}
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
    };

    template<class Skeleton> Improver::Improver(Skeleton&& skeleton, const std::vector<Case>& cases): cases(cases) {
        Algorithm algorithm(std::forward<Skeleton>(skeleton));
        algorithm.simplify();
        algorithm.normalize();
        this->skeleton = std::move(algorithm);
        this->fewest_moves = this->skeleton.length();
        this->skeleton_cube = Cube() * this->skeleton;
        this->inverse_skeleton_cube = Cube::inverse(this->skeleton_cube);
        this->init();
    }
};
