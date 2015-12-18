#ifndef INTERPROCESS_UTIL_HPP
#define INTERPROCESS_UTIL_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <iostream>

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

    class Log {
        enum class log_type {
            debug,
            warn,
            info,
            error
        };
        inline static void _log(const char *_s, log_type _type) {
            if (_type == log_type::error) {
                std::cerr << _s << std::endl;
            } else {
                std::cout << _s << std::endl;
            }
        }

        public:

        inline static void d(const char* _s) {_log(_s, log_type::debug);}
        inline static void w(const char* _s) {_log(_s, log_type::warn);}
        inline static void i(const char* _s) {_log(_s, log_type::info);}
        inline static void e(const char* _s) {_log(_s, log_type::error);}

        inline static void d(const std::string &_s) { Log::d(_s.c_str());}
        inline static void w(const std::string &_s) { Log::w(_s.c_str());}
        inline static void i(const std::string &_s) { Log::i(_s.c_str());}
        inline static void e(const std::string &_s) { Log::e(_s.c_str());}
    };
}
#endif //INTERPROCESS_UTIL_HPP
