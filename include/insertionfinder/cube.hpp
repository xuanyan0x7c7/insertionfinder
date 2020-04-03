#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <array>
#include <exception>
#include <functional>
#include <initializer_list>
#include <istream>
#include <optional>
#include <ostream>
#include <vector>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/twist.hpp>

namespace InsertionFinder {class Cube;};
template<> struct std::hash<InsertionFinder::Cube> {
    std::size_t operator()(const InsertionFinder::Cube& cube) const noexcept;
};

namespace InsertionFinder {
    struct CubeStreamError: std::exception {
        const char* what() const noexcept override {
            return "Failed to read cube from stream";
        }
    };

    namespace CubeTwist {
        constexpr std::byte corners {1};
        constexpr std::byte edges {2};
        constexpr std::byte centers {4};
        constexpr std::byte full = corners | edges | centers;
    };

    class Cube {
        friend struct std::hash<Cube>;
    private:
        class RawConstructor {};
        static constexpr RawConstructor raw_construct {};
    public:
        static constexpr int center_cycles[24] = {
            0, 2, 1, 2,
            2, 1, 3, 1,
            1, 3, 1, 3,
            2, 1, 3, 1,
            2, 1, 3, 1,
            2, 1, 3, 1
        };
    private:
        unsigned corner[8];
        unsigned edge[12];
        Rotation _placement;
    public:
        Cube() noexcept:
            corner {0, 3, 6, 9, 12, 15, 18, 21},
            edge {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
            _placement(0) {}
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
        static int compare(const Cube& lhs, const Cube& rhs) noexcept {
            return std::memcmp(&lhs, &rhs, sizeof(Cube));
        }
        bool operator==(const Cube& rhs) const noexcept {
            return std::memcmp(this, &rhs, sizeof(Cube)) == 0;
        }
        bool operator!=(const Cube& rhs) const noexcept {
            return std::memcmp(this, &rhs, sizeof(Cube)) != 0;
        }
    private:
        static const std::array<Cube, 24> rotation_cube;
        static const std::vector<std::array<int, 24>> corner_cycle_transform;
        static const std::vector<std::array<int, 24>> edge_cycle_transform;
    private:
        static std::array<Cube, 24> generate_rotation_cube_table() noexcept;
        static std::vector<std::array<int, 24>> generate_corner_cycle_transform_table();
        static std::vector<std::array<int, 24>> generate_edge_cycle_transform_table();
    public:
        void twist(const Algorithm& algorithm, std::byte flags = CubeTwist::full) noexcept {
            this->twist(algorithm, 0, algorithm.length(), flags);
            if (static_cast<bool>(flags & CubeTwist::centers)) {
                this->rotate(algorithm.cube_rotation());
            }
        }
        void twist_inverse(const Algorithm& algorithm, std::byte flags = CubeTwist::full) noexcept {
            if (static_cast<bool>(flags & CubeTwist::centers)) {
                this->rotate(algorithm.cube_rotation().inverse());
            }
            this->twist_inverse(algorithm, 0, algorithm.length(), flags);
        }
        void twist(const Algorithm& algorithm, std::size_t begin, std::size_t end, std::byte flags = CubeTwist::full) {
            for (std::size_t i = begin; i < end; ++i) {
                this->twist(algorithm[i], flags);
            }
        }
        void twist_inverse(
            const Algorithm& algorithm,
            std::size_t begin, std::size_t end,
            std::byte flags = CubeTwist::full
        ) {
            for (std::size_t i = end; i-- > begin;) {
                this->twist(algorithm[i].inverse(), flags);
            }
        }
        void twist(Twist twist, std::byte flags = CubeTwist::full) noexcept;
        void twist(const Cube& cube, std::byte flags = CubeTwist::full) noexcept;
        void twist_before(Twist twist, std::byte flags = CubeTwist::full) noexcept;
        void twist_before(const Cube& cube, std::byte flags = CubeTwist::full) noexcept;
        static Cube twist(const Cube& lhs, const Cube& rhs, std::byte lhs_flags, std::byte rhs_flags) noexcept;
        void rotate(Rotation rotation, std::byte flags = CubeTwist::full) noexcept {
            if (rotation) {
                this->twist(Cube::rotation_cube[rotation], flags);
            }
        }
    public:
        Cube operator*(const Algorithm& algorithm) const noexcept {
            Cube cube = *this;
            cube.twist(algorithm);
            return cube;
        }
        Cube operator*(Twist twist) const noexcept;
        Cube operator*(const Cube& rhs) const noexcept;
        friend Cube operator*(const Algorithm& algorithm, const Cube& rhs) noexcept {
            Cube cube;
            cube.twist(algorithm);
            cube.twist(rhs);
            return cube;
        }
        friend Cube operator*(Twist twist, const Cube& rhs) noexcept;
        template<class T> Cube& operator*=(const T& rhs) noexcept(noexcept(twist(rhs))) {
            this->twist(rhs);
            return *this;
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
        Rotation placement() const noexcept {
            return this->_placement;
        }
        Cube best_placement() const noexcept;
    private:
        static std::optional<Cube> corner_cycle_cube(unsigned index);
        static std::optional<Cube> edge_cycle_cube(unsigned index);
    public:
        int corner_cycle_index() const noexcept;
        int edge_cycle_index() const noexcept;
    public:
        static int next_corner_cycle_index(int index, Twist twist) {
            return Cube::corner_cycle_transform[index][twist];
        }
        static int next_corner_cycle_index(int index, std::initializer_list<Twist> twists) {
            for (Twist twist: twists) {
                index = Cube::corner_cycle_transform[index][twist];
            }
            return index;
        }
        static int next_edge_cycle_index(int index, Twist twist) {
            return Cube::edge_cycle_transform[index][twist];
        }
        static int next_edge_cycle_index(int index, std::initializer_list<Twist> twists) {
            for (Twist twist: twists) {
                index = Cube::edge_cycle_transform[index][twist];
            }
            return index;
        }
    };

    template<class T> Cube operator*(Cube&& lhs, const T& rhs) noexcept(noexcept(lhs.twist(rhs))) {
        lhs.twist(rhs);
        return std::move(lhs);
    }
};
