#include "clib.hpp"

inline bool clib::getBit(uint64_t x, uint8_t n) {
    return ((uint64_t)x & ((uint64_t)1 << n)) == ((uint64_t)1 << n);
}
