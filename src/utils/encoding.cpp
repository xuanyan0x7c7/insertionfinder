#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>
#include "encoding.hpp"
using std::uint64_t;
using std::uint8_t;


void InsertionFinder::Details::write_varuint(std::ostream& out, uint64_t n) {
    uint8_t buffer[8];
    for (size_t i = 0; i < 8; ++i) {
        buffer[i] = n >> (i << 3) & 0xff;
    }
    if (n < 0xfd) {
        out.write(reinterpret_cast<char*>(&buffer), 1);
    } else if (n <= 0xffff) {
        char flag = 0xfd;
        out.write(&flag, 1);
        out.write(reinterpret_cast<char*>(&buffer), 2);
    } else if (n <= 0xffffffffu) {
        char flag = 0xfe;
        out.write(&flag, 1);
        out.write(reinterpret_cast<char*>(&buffer), 4);
    } else {
        char flag = 0xff;
        out.write(&flag, 1);
        out.write(reinterpret_cast<char*>(&buffer), 8);
    }
}

std::optional<uint64_t> InsertionFinder::Details::read_varuint(std::istream& in) {
    uint8_t flag;
    in.read(reinterpret_cast<char*>(&flag), 1);
    if (in.gcount() != 1) {
        return {};
    }
    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (flag < 0xfd) {
        buffer[0] = flag;
    } else if (flag == 0xfd) {
        in.read(reinterpret_cast<char*>(&buffer), 2);
        if (in.gcount() != 2) {
            return {};
        }
    } else if (flag == 0xfe) {
        in.read(reinterpret_cast<char*>(&buffer), 4);
        if (in.gcount() != 4) {
            return {};
        }
    } else {
        in.read(reinterpret_cast<char*>(&buffer), 8);
        if (in.gcount() != 8) {
            return {};
        }
    }
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= static_cast<uint64_t>(buffer[i]) << (i << 3);
    }
    return result;
}
