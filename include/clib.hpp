#ifndef GBEMU_CLIB_H
#define GBEMU_CLIB_H

#include <cstdint>

namespace clib {
    uint64_t upow(uint64_t x, uint64_t n);
    uint8_t getBit(uint64_t x, uint8_t n);
}  // namespace clib

#endif