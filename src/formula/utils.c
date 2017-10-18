#include <stddef.h>
#include <stdint.h>
#include "utils.h"


uint32_t formula_move_mask(int move) {
    return 1 << move | 1 << (24 + (move >> 2));
}


size_t formula_get_min_capacity(size_t length) {
    size_t capacity = 32;
    while ((capacity <<= 1) < length);
    return capacity;
}
