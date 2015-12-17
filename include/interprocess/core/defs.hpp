#ifndef INTERPROCESS_DEFS_HPP
#define INTERPROCESS_DEFS_HPP

#include <cstdint>
#include <sstream>
#include "util.hpp"

namespace interproc {
    using block_descriptor_t = uint32_t;
    using byte_t = uint8_t;
    using ibufstream = std::basic_istringstream<byte_t>;
    using obufstream = std::basic_ostringstream<byte_t>;

    const uint32_t BLOCK_DESCRIPTOR_SIZE = sizeof(block_descriptor_t);

    enum class protocol: symbol_t {
        ipc = symbol("ipc"),
        tcp = symbol("tcp"),
        unix = symbol("unix")
    };
}

#endif //INTERPROCESS_DEFS_HPP
