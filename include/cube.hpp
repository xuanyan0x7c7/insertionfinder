#pragma once
#include <cstdint>
#include <exception>
#include <istream>
#include <optional>
#include <ostream>

namespace InsertionFinder {
    struct CubeStreamError: std::exception {
        virtual const char* what() const noexcept override {
            return "Failed to read cube from stream";
        }
    };

    class Cube {
    private:
        int corner[8];
        int edge[12];
    public:
        Cube() noexcept;
        Cube(const Cube&) = default;
        Cube(Cube&&) = default;
        Cube& operator=(const Cube&) = default;
        Cube& operator=(Cube&&) = default;
        ~Cube() = default;
        explicit Cube(std::nullopt_t) noexcept {}
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        Cube inverse() const noexcept;
        std::uint32_t mask() const noexcept;
    };
};
