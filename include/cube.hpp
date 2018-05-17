#pragma once
#include <cstdint>
#include <array>
#include <exception>
#include <functional>
#include <istream>
#include <ostream>
#include <utility>
#include <fallbacks/optional.hpp>
#include <algorithm.hpp>

namespace InsertionFinder {class Cube;};
template<> struct std::hash<InsertionFinder::Cube> {
    std::size_t operator()(const InsertionFinder::Cube& cube) const noexcept;
};

namespace InsertionFinder {
    struct CubeStreamError: std::exception {
        virtual const char* what() const noexcept override {
            return "Failed to read cube from stream";
        }
    };

    namespace CubeTwist {
        constexpr std::uint8_t corners = 1;
        constexpr std::uint8_t edges = 2;
        constexpr std::uint8_t centers = 4;
        constexpr std::uint8_t reversed = 8;
    };

    class Cube {
        friend struct std::hash<Cube>;
    private:
        int corner[8];
        int edge[12];
        int center;
    public:
        Cube() noexcept;
        Cube(const Cube&) = default;
        Cube(Cube&&) = default;
        Cube& operator=(const Cube&) = default;
        Cube& operator=(Cube&&) = default;
        ~Cube() = default;
    private:
        explicit Cube(std::nullopt_t) noexcept {}
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        static int compare(const Cube& lhs, const Cube& rhs) noexcept;
        bool operator==(const Cube& rhs) const noexcept {
            return this->compare(*this, rhs) == 0;
        }
        bool operator<(const Cube& rhs) const noexcept {
            return this->compare(*this, rhs) < 0;
        }
    private:
        static const std::array<Cube, 24> twist_cube;
        static const std::array<Cube, 24> rotation_cube;
        static const std::array<std::array<int, 24>, 7 * 24 * 11 * 24>
            parity_transform;
        static const std::array<std::array<int, 24>, 6 * 24 * 24>
            corner_cycle_transform;
        static const std::array<std::array<int, 24>, 10 * 24 * 24>
            edge_cycle_transform;
        static const std::array<std::array<int, 24>, 24> center_transform;
    private:
        static std::array<Cube, 24> generate_twist_cube_table() noexcept;
        static std::array<Cube, 24> generate_rotation_cube_table() noexcept;
        static std::array<std::array<int, 24>, 7 * 24 * 11 * 24>
        generate_parity_transform_table() noexcept;
        static std::array<std::array<int, 24>, 6 * 24 * 24>
        generate_corner_cycle_transform_table() noexcept;
        static std::array<std::array<int, 24>, 10 * 24 * 24>
        generate_edge_cycle_transform_table() noexcept;
    public:
        void twist(
            int twist,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        );
        void twist(
            const Algorithm& algorithm,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        void twist(
            const Algorithm& algorithm,
            std::size_t begin, std::size_t end,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        void twist(
            const Cube& cube,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        void twist_before(
            int twist,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        );
        void twist_before(
            const Cube& cube,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        std::optional<Cube> twist_effectively(
            const Cube& cube,
            std::uint8_t flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) const noexcept;
        void rotate(int rotation);
    public:
        Cube inverse() const noexcept;
        std::uint32_t mask() const noexcept;
    public:
        bool has_parity() const noexcept;
        int corner_cycles() const noexcept;
        int edge_cycles() const noexcept;
        int center_rotation() const noexcept {
            return this->center;
        }
    private:
        static Cube parity_cube(int index);
        static Cube corner_cycle_cube(int index);
        static Cube edge_cycle_cube(int index);
    public:
        int parity_index() const noexcept;
        int corner_cycle_index() const noexcept;
        int edge_cycle_index() const noexcept;
        static bool center_parity(int rotation);
        std::pair<int, Cube> best_center_rotation() const noexcept;
    public:
        static int next_parity_index(int index, int twist) {
            return Cube::parity_transform[index][twist];
        }
        static int next_corner_cycle_index(int index, int twist) {
            return Cube::corner_cycle_transform[index][twist];
        }
        static int next_edge_cycle_index(int index, int twist) {
            return Cube::edge_cycle_transform[index][twist];
        }
    };
};
