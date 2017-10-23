#pragma once
#include <cstdint>
#include <exception>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace InsertionFinder {class Algorithm;};
std::ostream& operator<<(std::ostream&, const InsertionFinder::Algorithm&);

namespace InsertionFinder {
    class AlgorithmError: public std::exception {
    private:
        const std::string algorithm_string;
        const std::string explanation_string;
    public:
        AlgorithmError(std::string algorithm_string):
            algorithm_string(std::move(algorithm_string)),
            explanation_string(
                "Invalid algorithm string: " + this->algorithm_string
            ) {}
    public:
        virtual const char* what() const noexcept override {
            return explanation_string.c_str();
        }
    };

    struct AlgorithmStreamError: std::exception {
        virtual const char* what() const noexcept override {
            return "Failed to read algorithm from stream";
        }
    };

    class Algorithm {
    public:
        static constexpr int inverse_twist[24] = {
            0, 3, 2, 1,
            4, 7, 6, 5,
            8, 11, 10, 9,
            12, 15, 14, 13,
            16, 19, 18, 17,
            20, 23, 22, 21
        };
    private:
        std::vector<int> twists;
    private:
        std::uint32_t begin_mask;
        std::uint32_t end_mask;
        std::uint32_t set_up_mask;
    public:
        Algorithm() {}
        Algorithm(const Algorithm&) = default;
        Algorithm(Algorithm&&) = default;
        Algorithm& operator=(const Algorithm&) = default;
        Algorithm& operator=(Algorithm&&) = default;
        ~Algorithm() = default;
        explicit Algorithm(const std::string& algorithm_string);
    public:
        std::size_t length() const noexcept {
            return this->twists.size();
        }
        int operator[](std::size_t index) const {
            return this->twists[index];
        }
    public:
        static int compare(const Algorithm& lhs, const Algorithm& rhs) noexcept;
        bool operator==(const Algorithm& rhs) const noexcept {
            return this->compare(*this, rhs) == 0;
        }
        bool operator<(const Algorithm& rhs) const noexcept {
            return this->compare(*this, rhs) < 0;
        }
    public:
        friend std::ostream&
        ::operator<<(std::ostream& out, const Algorithm& algorithm);
        void print(std::ostream& out, std::size_t begin, std::size_t end) const;
        std::string to_string() const;
    public:
        void save_to(std::ostream& out) const;
        void read_from(std::istream& in);
    private:
        std::size_t cancel_moves();
    public:
        std::pair<std::uint32_t, std::uint32_t>
        get_insert_place_mask(std::size_t insert_place) const;
        std::pair<Algorithm, std::size_t>
        insert(const Algorithm& insertion, std::size_t insert_place) const;
        bool is_worthy_insertion(
            const Algorithm& insertion, std::size_t insert_place,
            const std::pair<std::uint32_t, std::uint32_t>& insert_place_mask,
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
        void normalize() noexcept;
        std::vector<Algorithm> generate_isomorphisms() const;
    };
};
