#pragma once
#include <cstdint>
#include <array>
#include <exception>
#include <istream>
#include <optional>
#include <ostream>
#include <algorithm.hpp>

namespace InsertionFinder {
    struct CubeStreamError: std::exception {
        virtual const char* what() const noexcept override {
            return "Failed to read cube from stream";
        }
    };

    namespace CubeTwist {
        constexpr std::uint8_t corners = 1;
        constexpr std::uint8_t edges = 2;
        constexpr std::uint8_t reversed = 4;
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
    private:
        static const std::array<Cube, 24> twist_cube;
    private:
        std::array<Cube, 24> generate_twist_cube_table() noexcept;
    public:
        void twist(
            int twist,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        );
        void twist(
            const Algorithm& algorithm,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        ) noexcept;
        void twist(
            const Algorithm& algorithm,
            std::size_t begin, std::size_t end,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        ) noexcept;
        void twist(
            const Cube& cube,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        ) noexcept;
        void twist_before(
            int twist,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        );
        void twist_before(
            const Cube& cube,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        ) noexcept;
        std::optional<Cube> twist_effectively(
            const Cube& cube,
            std::uint8_t flags = CubeTwist::corners | CubeTwist::edges
        ) const noexcept;
    public:
        Cube inverse() const noexcept;
        std::uint32_t mask() const noexcept;
    };
};
