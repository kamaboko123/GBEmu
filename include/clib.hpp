#ifndef GBEMU_CLIB_H
#define GBEMU_CLIB_H

#include <cstdint>

namespace clib {
    bool getBit(uint64_t x, uint8_t n);
}  // namespace clib

#endif