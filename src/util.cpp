#include "../include/interprocess/core/util.hpp"
#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <sys/types.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <process.h>
#endif

uint64_t interproc::symbol(const std::string &str) {
    return (!str.c_str() ? 0 : symbol_recursive(5381, str.c_str()));
}

uint32_t interproc::getmypid() {
#if defined(__linux__) || defined(__APPLE__)
    return getpid();
#elif defined(_WIN32)
    return _getpid();
    #else
    return 0; // This should not happen
#endif
}
