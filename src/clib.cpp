#include "clib.hpp"

uint64_t clib::upow(uint64_t x, uint64_t n) {
    if (n == 0) return (1);
    if (n == 1) return (x);
    return (x * upow(x, n - 1));
}

uint8_t clib::getBit(uint64_t x, uint8_t n) {
    return ((uint64_t)(x & upow(2, n)) >> n);
}
