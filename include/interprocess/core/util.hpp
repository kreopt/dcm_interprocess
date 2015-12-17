#ifndef INTERPROCESS_UTIL_HPP
#define INTERPROCESS_UTIL_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace interproc {
    using symbol_t = uint64_t ;

    namespace {
        constexpr uint64_t symbol_recursive(unsigned int hash, const char *str) {
            return (!*str ? hash : symbol_recursive(((hash << 5) + hash) + *str, str + 1));
        }
    }

    constexpr uint64_t symbol(const char* str) {
        return ( !str ? 0 : symbol_recursive(5381, str));
    }

    uint64_t symbol(const std::string &str) {
        return ( !str.c_str() ? 0 : symbol_recursive(5381, str.c_str()));
    }

    symbol_t constexpr operator "" _sym(const char* s, size_t) {
        return symbol(s);
    }
}
#endif //INTERPROCESS_UTIL_HPP
