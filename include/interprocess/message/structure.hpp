#ifndef INTERPROC_MESSAGE_HPP
#define INTERPROC_MESSAGE_HPP

#include <string>
#include <initializer_list>

namespace interproc {
    class structure {
    public:
        virtual ~structure(){};
        virtual interproc::buffer to_buffer() const = 0;
        virtual void from_buffer(const interproc::buffer &_buf) = 0;
        virtual void from_buffer(interproc::buffer &&_buf) = 0;

        template <typename ValueType>
        virtual void set(const interproc::symbol_t _key, ValueType &&_val) = 0;

        template <typename ValueType>
        virtual void set(const interproc::symbol_t _key, const ValueType &_val) = 0;

        template <typename ValueType>
        virtual void get(const interproc::symbol_t _key, ValueType &&_default) = 0;

        template <typename ValueType>
        virtual void set_value(std::initializer_list<std::pair<interproc::symbol_t, ValueType>> &_il) = 0;
    };


}

#endif