#pragma once
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <insertionfinder/twist.hpp>

namespace InsertionFinder {class Algorithm;};
std::ostream& operator<<(std::ostream&, const InsertionFinder::Algorithm&);
template<> struct std::hash<InsertionFinder::Algorithm> {
    std::size_t operator()(const InsertionFinder::Algorithm& algorithm) const noexcept;
};

namespace InsertionFinder {
    class AlgorithmError: public std::exception {
    private:
        const std::string algorithm_string;
        const std::string explanation_string;
    public:
        template<class T> AlgorithmError(T&& algorithm_string):
            algorithm_string(std::forward<T>(algorithm_string)),
            explanation_string("Invalid algorithm string: " + this->algorithm_string) {}
    public:
        const char* what() const noexcept override {
            return explanation_string.c_str();
        }
    };

    struct AlgorithmStreamError: std::exception {
        const char* what() const noexcept override {
            return "Failed to read algorithm from stream";
        }
    };

    class InsertionAlgorithm;

    class Algorithm {
        friend struct std::hash<Algorithm>;
    protected:
        std::vector<Twist> twists;
        Rotation rotation;
    public:
        Algorithm(): rotation(0) {}
        Algorithm(const Algorithm&) = default;
        Algorithm(Algorithm&&) = default;
        Algorithm& operator=(const Algorithm&) = default;
        Algorithm& operator=(Algorithm&&) = default;
        ~Algorithm() = default;
        explicit Algorithm(const char* algorithm_string);
        explicit Algorithm(const std::string& algorithm_string): Algorithm(algorithm_string.data()) {}
    public:
        std::size_t length() const noexcept {
            return this->twists.size();
        }
        Twist operator[](std::size_t index) const {
            return this->twists[index];
        }
        Rotation cube_rotation() const noexcept {
            return this->rotation;
        }
    public:
        static int compare(const Algorithm& lhs, const Algorithm& rhs) noexcept;
        bool operator==(const Algorithm& rhs) const noexcept {
            return this->rotation == rhs.rotation && this->twists == rhs.twists;
        }
        bool operator!=(const Algorithm& rhs) const noexcept {
            return this->rotation != rhs.rotation || this->twists != rhs.twists;
        }
        bool operator<(const Algorithm& rhs) const noexcept {
            return Algorithm::compare(*this, rhs) < 0;
        }
        bool operator<=(const Algorithm& rhs) const noexcept {
            return Algorithm::compare(*this, rhs) <= 0;
        }
        bool operator>(const Algorithm& rhs) const noexcept {
            return Algorithm::compare(*this, rhs) > 0;
        }
        bool operator>=(const Algorithm& rhs) const noexcept {
            return Algorithm::compare(*this, rhs) >= 0;
        }
    public:
        friend std::ostream& ::operator<<(std::ostream& out, const Algorithm& algorithm);
        void print(std::ostream& out, std::size_t begin, std::size_t end) const;
        void print(std::ostream& out, const std::vector<int>& marks) const;
        void print(std::ostream& out, const std::vector<int>& marks, std::size_t begin, std::size_t end) const;
        std::string str() const;
    public:
        void save_to(std::ostream& out) const;
        virtual void read_from(std::istream& in);
    public:
        Algorithm operator+(const Algorithm& rhs) const;
        template<class T> Algorithm operator+(T&& rhs) const {
            return *this + Algorithm(std::forward<T>(rhs));
        }
        friend Algorithm operator+(Algorithm&& lhs, const Algorithm& rhs) {
            return std::move(lhs += rhs);
        }
        template<class T> friend Algorithm operator+(T&& lhs, const Algorithm& rhs) {
            return Algorithm(std::forward<T>(lhs)) + rhs;
        }
        Algorithm& operator+=(const Algorithm& rhs);
        template<class T> Algorithm& operator+=(T&& rhs) {
            return *this += Algorithm(std::forward<T>(rhs));
        }
    private:
        std::size_t cancel_moves() noexcept;
        std::vector<int> cancel_moves_return_marks();
    public:
        void detect_rotation() noexcept;
        std::pair<std::uint32_t, std::uint32_t> get_insert_place_mask(std::size_t insert_place) const;
        std::pair<Algorithm, std::size_t> insert(const Algorithm& insertion, std::size_t insert_place) const;
        std::tuple<Algorithm, std::vector<int>, std::vector<int>>
        insert_return_marks(const Algorithm& insertion, std::size_t insert_place) const;
        std::tuple<Algorithm, std::vector<int>, std::vector<std::vector<int>>>
        multi_insert_return_marks(
            const std::vector<Algorithm>& insertions,
            const std::vector<std::pair<std::size_t, std::vector<std::size_t>>>& insert_places
        ) const;
        bool is_worthy_insertion(
            const InsertionAlgorithm& insertion, std::size_t insert_place,
            std::pair<std::uint32_t, std::uint32_t> insert_place_mask,
            std::size_t fewest_twists = std::numeric_limits<std::size_t>::max()
        ) const;
    public:
        bool swappable(std::size_t place) const {
            return place > 0 && place < this->twists.size()
                && this->twists[place - 1] >> 3 == this->twists[place] >> 3;
        }
        void swap_adjacent(std::size_t place) {
            std::swap(this->twists[place - 1], this->twists[place]);
        }
    public:
        void simplify() noexcept {
            this->rotation = 0;
            this->cancel_moves();
        }
        void normalize() noexcept;
        void rotate(Rotation rotation);
        void inverse() noexcept;
        static Algorithm inverse(const Algorithm& algorithm);
        std::vector<Algorithm> generate_symmetrics() const;
        std::vector<Algorithm> generate_rotation_conjugates() const;
        std::vector<Algorithm> generate_similars() const;
    };

    inline Algorithm operator""_alg(const char* string, std::size_t length) {
        return Algorithm(string);
    }

    class InsertionAlgorithm: public Algorithm {
        friend class Algorithm;
    private:
        std::uint32_t begin_mask;
        std::uint32_t end_mask;
        std::uint32_t set_up_mask;
    public:
        InsertionAlgorithm(): Algorithm() {}
        InsertionAlgorithm(const InsertionAlgorithm&) = default;
        InsertionAlgorithm(InsertionAlgorithm&&) = default;
        InsertionAlgorithm& operator=(const InsertionAlgorithm&) = default;
        InsertionAlgorithm& operator=(InsertionAlgorithm&&) = default;
        ~InsertionAlgorithm() = default;
        InsertionAlgorithm(const Algorithm& algorithm): Algorithm(algorithm) {}
        InsertionAlgorithm(Algorithm&& algorithm): Algorithm(std::move(algorithm)) {}
    public:
        void read_from(std::istream& in) override;
    };
};
