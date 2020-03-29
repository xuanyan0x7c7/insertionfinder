#pragma once
#include <istream>
#include <optional>
#include <ostream>

namespace InsertionFinder::Details {
    void write_varuint(std::ostream& out, std::uint64_t n);
    std::optional<std::uint64_t> read_varuint(std::istream& in);
};
