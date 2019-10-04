#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
#include <exception>
#include <functional>
#include <istream>
#include <ostream>
#include <vector>
#include <insertionfinder/algorithm.hpp>

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
        constexpr std::byte full = corners | edges | centers;
    };

    class Cube {
        friend struct std::hash<Cube>;
    private:
        class RawConstructor {};
        static constexpr RawConstructor raw_construct {};
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
        explicit Cube(RawConstructor) noexcept {}
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    public:
        static void init();
    public:
        static int compare(const Cube& lhs, const Cube& rhs) noexcept;
        bool operator==(const Cube& rhs) const noexcept;
        bool operator<(const Cube& rhs) const noexcept {
            return this->compare(*this, rhs) < 0;
        }
    private:
        static std::array<Cube, 24> twist_cube;
        static std::array<Cube, 24> rotation_cube;
        static std::vector<std::array<int, 24>> corner_cycle_transform;
        static std::vector<std::array<int, 24>> edge_cycle_transform;
    public:
        static const std::array<std::array<int, 24>, 24> center_transform;
    private:
        static void generate_twist_cube_table() noexcept;
        static void generate_rotation_cube_table() noexcept;
        static void generate_corner_cycle_transform_table() noexcept;
        static void generate_edge_cycle_transform_table() noexcept;
    public:
        void twist(int twist, std::byte flags = CubeTwist::full) {
            this->twist(Cube::twist_cube[twist], flags);
        }
        void twist(const Algorithm& algorithm, std::byte flags = CubeTwist::full) noexcept {
            this->twist(algorithm, 0, algorithm.length(), flags);
        }
        void twist(const Algorithm& algorithm, std::size_t begin, std::size_t end, std::byte flags = CubeTwist::full);
        void twist(const Cube& cube, std::byte flags = CubeTwist::full) noexcept;
        void twist_before(int twist, std::byte flags = CubeTwist::full) {
            this->twist_before(Cube::twist_cube[twist], flags);
        }
        void twist_before(const Cube& cube, std::byte flags = CubeTwist::full) noexcept;
        static Cube twist(const Cube& lhs, const Cube& rhs, std::byte flags = CubeTwist::full) noexcept;
        void rotate(int rotation) {
            if (rotation) {
                this->twist(Cube::rotation_cube[rotation]);
            }
        }
        static int placement_twist(int placement, int rotation) {
            return Cube::center_transform[placement][rotation];
        }
    public:
        void inverse() noexcept;
        static Cube inverse(const Cube& cube) noexcept;
        std::uint32_t mask() const noexcept;
    public:
        bool has_parity() const noexcept;
        bool parity() const noexcept {
            return Cube::center_cycles[this->_placement] > 1 || this->has_parity();
        }
        int corner_cycles() const noexcept;
        int edge_cycles() const noexcept;
        int placement() const noexcept {
            return this->_placement;
        }
    private:
        static Cube corner_cycle_cube(int index);
        static Cube edge_cycle_cube(int index);
    public:
        int corner_cycle_index() const noexcept;
        int edge_cycle_index() const noexcept;
        Cube best_placement() const noexcept;
    public:
        static int next_corner_cycle_index(int index, int twist) {
            return Cube::corner_cycle_transform[index][twist];
        }
        static int next_edge_cycle_index(int index, int twist) {
            return Cube::edge_cycle_transform[index][twist];
        }
    };
};
