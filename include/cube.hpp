#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
#include <exception>
#include <functional>
#include <istream>
#include <ostream>
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
        constexpr std::byte corners {1};
        constexpr std::byte edges {2};
        constexpr std::byte centers {4};
        constexpr std::byte reversed {8};
    };

    class Cube {
        friend struct std::hash<Cube>;
    public:
        static constexpr int inverse_center[24] = {
            0, 3, 2, 1,
            12, 23, 6, 17,
            8, 9, 10, 11,
            4, 19, 14, 21,
            20, 7, 18, 13,
            16, 15, 22, 5
        };
        static constexpr int center_cycles[24] = {
            0, 2, 1, 2,
            2, 1, 3, 1,
            1, 3, 1, 3,
            2, 1, 3, 1,
            2, 1, 3, 1,
            2, 1, 3, 1
        };
    private:
        int corner[8];
        int edge[12];
        int _placement;
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
        static void init();
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
        static std::array<Cube, 24> rotation_cube;
        static std::array<std::array<int, 24>, 7 * 24 * 11 * 24>
            parity_transform;
        static std::array<std::array<int, 24>, 6 * 24 * 24>
            corner_cycle_transform;
        static std::array<std::array<int, 24>, 10 * 24 * 24>
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
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        );
        void twist(
            const Algorithm& algorithm,
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        void twist(
            const Algorithm& algorithm,
            std::size_t begin, std::size_t end,
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        );
        void twist(
            const Cube& cube,
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        void twist_before(
            int twist,
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        );
        void twist_before(
            const Cube& cube,
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) noexcept;
        std::optional<Cube> twist_effectively(
            const Cube& cube,
            std::byte flags =
                CubeTwist::corners | CubeTwist::edges | CubeTwist::centers
        ) const noexcept;
        void rotate(int rotation);
        static int placement_twist(int placement, int rotation) {
            return Cube::center_transform[placement][rotation];
        }
    public:
        Cube inverse() const noexcept;
        std::uint32_t mask() const noexcept;
    public:
        bool has_parity() const noexcept;
        int corner_cycles() const noexcept;
        int edge_cycles() const noexcept;
        int placement() const noexcept {
            return this->_placement;
        }
    private:
        static Cube parity_cube(int index);
        static Cube corner_cycle_cube(int index);
        static Cube edge_cycle_cube(int index);
    public:
        int parity_index() const noexcept;
        int corner_cycle_index() const noexcept;
        int edge_cycle_index() const noexcept;
        static bool placement_parity(int rotation);
        Cube best_placement() const noexcept;
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
