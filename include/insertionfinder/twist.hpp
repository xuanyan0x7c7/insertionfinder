#pragma once
#include <cstdint>
#include <array>
#include <ostream>
#include <string>

namespace InsertionFinder {
    class Rotation;
    class Twist;
};
std::ostream& operator<<(std::ostream&, InsertionFinder::Rotation);
std::ostream& operator<<(std::ostream&, InsertionFinder::Twist);

namespace InsertionFinder {
    class Rotation {
    private:
        static constexpr int inverse_rotation[24] = {
            0, 3, 2, 1,
            12, 23, 6, 17,
            8, 9, 10, 11,
            4, 19, 14, 21,
            20, 7, 18, 13,
            16, 15, 22, 5
        };
        static constexpr int rotation_permutation[24][3] = {
            {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
            {5, 2, 0}, {2, 4, 0}, {4, 3, 0}, {3, 5, 0},
            {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
            {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1},
            {2, 1, 4}, {4, 1, 3}, {3, 1, 5}, {5, 1, 2},
            {3, 0, 4}, {5, 0, 3}, {2, 0, 5}, {4, 0, 2}
        };
        static const std::array<std::array<int, 24>, 24> rotation_transform;
    private:
        int rotation;
    public:
        Rotation() = default;
        constexpr Rotation(const Rotation&) = default;
        constexpr Rotation(Rotation&&) = default;
        Rotation& operator=(const Rotation&) = default;
        Rotation& operator=(Rotation&&) = default;
        ~Rotation() = default;
    public:
        constexpr Rotation(int rotation): rotation(rotation) {}
    public:
        friend std::ostream& ::operator<<(std::ostream& out, Rotation rotation);
        std::string str() const;
    private:
        static std::array<std::array<int, 24>, 24> generate_rotation_transform_table() noexcept;
    public:
        constexpr Rotation inverse() const noexcept {
            return Rotation::inverse_rotation[this->rotation];
        }
        constexpr static int inverse(int rotation) {
            return Rotation::inverse_rotation[rotation];
        }
        constexpr operator int() const noexcept {
            return this->rotation;
        }
        void rotate(Rotation rotation) noexcept {
            *this *= rotation;
        }
        friend Rotation operator*(Rotation lhs, Rotation rhs) noexcept {
            return Rotation::rotation_transform[lhs.rotation][rhs.rotation];
        }
        Rotation& operator*=(Rotation rotation) noexcept {
            return *this = *this * rotation;
        }
    };

    class Twist {
    private:
        static constexpr std::uint_fast8_t inverse_twist[24] = {
            0, 3, 2, 1,
            4, 7, 6, 5,
            8, 11, 10, 9,
            12, 15, 14, 13,
            16, 19, 18, 17,
            20, 23, 22, 21
        };
        static constexpr std::uint_fast8_t rotation_permutation[24][3] = {
            {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
            {5, 2, 0}, {2, 4, 0}, {4, 3, 0}, {3, 5, 0},
            {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
            {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1},
            {2, 1, 4}, {4, 1, 3}, {3, 1, 5}, {5, 1, 2},
            {3, 0, 4}, {5, 0, 3}, {2, 0, 5}, {4, 0, 2}
        };
    private:
        std::uint_fast8_t twist;
    public:
        Twist() = default;
        constexpr Twist(const Twist&) = default;
        constexpr Twist(Twist&&) = default;
        Twist& operator=(const Twist&) = default;
        Twist& operator=(Twist&&) = default;
        ~Twist() = default;
    public:
        constexpr Twist(std::uint_fast8_t twist): twist(twist) {}
        Twist(const char* string);
        Twist(const std::string& string): Twist(string.data()) {}
    public:
        friend std::ostream& ::operator<<(std::ostream& out, Twist twist);
        std::string str() const;
    public:
        constexpr Twist inverse() const noexcept {
            return Twist::inverse_twist[this->twist];
        }
        constexpr static std::uint_fast8_t inverse(std::uint_fast8_t twist) {
            return Twist::inverse_twist[twist];
        }
        constexpr operator std::uint_fast8_t() const noexcept {
            return this->twist;
        }
        void rotate(Rotation rotation) noexcept {
            *this *= rotation;
        }
        Twist operator*(Rotation rotation) const noexcept {
            const std::uint_fast8_t* transform = Twist::rotation_permutation[rotation];
            return transform[this->twist >> 3] << 2 ^ (this->twist & 7);
        }
        Twist& operator*=(Rotation rotation) noexcept {
            return *this = *this * rotation;
        }
    };
};
