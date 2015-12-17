#ifndef INTERPROCESS_BUFFER_HPP
#define INTERPROCESS_BUFFER_HPP

#include <string>
#include <type_traits>
#include "defs.hpp"

namespace interproc {
    using buffer = std::basic_string<byte_t>;

    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    inline buffer to_buffer(T v){
        return buffer(reinterpret_cast<byte_t*>(v), sizeof(T));
    }

    template <>
    inline buffer to_buffer(bool v){
        byte_t b = v?'\0':'\1';
        return buffer(&b, 1);
    }

    inline buffer to_buffer(const std::string &_s){
        return buffer(reinterpret_cast<const byte_t*>(_s.data()), _s.size());
    }

    inline void write_size(obufstream& _s, block_descriptor_t _sz){
        _s.write(reinterpret_cast<interproc::byte_t*>(&_sz), interproc::BLOCK_DESCRIPTOR_SIZE);
    }

    inline void read_size(ibufstream& _s, block_descriptor_t &_sz){
        byte_t* lb = reinterpret_cast<byte_t*>(&_sz);
        _s.read(lb, interproc::BLOCK_DESCRIPTOR_SIZE);
    }
}

#endif //INTERPROCESS_BUFFER_HPP
