#include <stdint.h>
#include "utils.h"

uint32_t MoveMask(int move) {
    return (1 << move) | (1 << (24 + (move >> 2)));
}
