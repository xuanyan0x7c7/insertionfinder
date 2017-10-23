#pragma once
#include <cstdint>
#include <exception>
#include <istream>
#include <ostream>
#include <vector>
#include <algorithm.hpp>
#include <cube.hpp>

namespace InsertionFinder {
    struct CaseStreamError: std::exception {
        virtual const char* what() const noexcept override {
            return "Failed to read case from stream";
        }
    };

    class Case {
    private:
        Cube state;
        std::vector<Algorithm> algorithm_list;
    private:
        std::uint32_t mask;
        bool has_parity;
        int corner_cycles;
        int edge_cycles;
    public:
        Case(const Cube& state);
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    };
};
