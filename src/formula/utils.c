#include <stdint.h>
#include "utils.h"

uint32_t move_mask(int move) {
    return 1 << move | 1 << (24 + (move >> 2));
}
