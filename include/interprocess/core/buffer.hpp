#ifndef INTERPROCESS_BUFFER_HPP
#define INTERPROCESS_BUFFER_HPP

#include <string>
#include <cstring>
#include <type_traits>
#include "defs.hpp"

namespace interproc {
//    using buffer = std::basic_string<byte_t>;
    class buffer {
        char *data_;
        size_t size_;
        bool wrapped_ = false;
    public:

        inline size_t size() const { return size_; }

        inline const char *data() const { return data_; }
        inline char *data() { return data_; }

        virtual ~buffer() {
            if (!wrapped_) {
                delete[] data_;
            }
        }

        buffer(const buffer& _buf) {
            size_ = _buf.size_;
            data_ = new char[size_];
            memcpy(data_, _buf.data_, size_);
        }

        buffer(buffer&& _buf) {
            size_ = _buf.size_;
            data_ = _buf.data_;
            _buf.data_ = nullptr;
            _buf.size_ = 0;
        }

        buffer() {
            size_ = 0;
            data_ = nullptr;
        }

        buffer(const char *_data) {
            size_ = sizeof(_data);
            data_ = new char[size_];
            memcpy(data_, _data, size_);
        }

        buffer(const char *_data, size_t _size, bool _wrap = false) {
            size_ = _size;
            if (_wrap) {
                data_ = const_cast<char*>(_data);
                wrapped_=true;
            } else {
                data_ = new char[_size];
                memcpy(data_, _data, size_);
            }
        }

        buffer(bool _val) {
            size_ = 1;
            data_ = new char[1]{static_cast<char>(_val ? 1 : 0)};
//            data_[0] = _val ? 1 : 0;
        }

        template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        buffer(T _val) {
            size_ = sizeof(T);
            data_ = new char[size_];
            memcpy(const_cast<char *>(data_), reinterpret_cast<char *>(_val), size_);
        };

        buffer(const std::string &_data, bool _wrap = false) {
            size_ = _data.size();
            if (_wrap) {
                wrapped_ = true;
                data_ = const_cast<char*>(&_data.front());
            } else {
                data_ = new char[size_];
                memcpy(const_cast<char *>(data_), _data.data(), size_);
            }
        }


        buffer& operator=(const buffer& _buf) {
            size_ = _buf.size_;
            if (data_) {
                delete [] data_;
            }
            data_ = new char[size_];
            memcpy(data_, _buf.data_, size_);
            return *this;
        }

        buffer& operator=(buffer&& _buf) {
            size_ = _buf.size_;
            if (data_) {
                delete [] data_;
            }
            data_ = _buf.data_;
            _buf.data_ = nullptr;
            _buf.size_ = 0;
            return *this;
        }

        inline void clear() {
            size_=0;
            if (!wrapped_) {
                delete[] data_;
            }
            data_= nullptr;
        }

        inline void resize(size_t _size) {
            if (data_) clear();
            size_=_size;
            data_ = new char[size_];
        }
    };
//
//    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
//    inline buffer to_buffer(T v){
//        return buffer(reinterpret_cast<byte_t*>(v), sizeof(T));
//    }
//
//    template <>
//    inline buffer to_buffer(bool v){
//        byte_t b = v?'\0':'\1';
//        return buffer(&b, 1);
//    }
//
//    inline buffer to_buffer(const std::string &_s){
//        return buffer(reinterpret_cast<const byte_t*>(_s.data()), _s.size());
//    }

    inline void write_size(obufstream& _s, block_descriptor_t _sz){
        _s.write(reinterpret_cast<interproc::byte_t*>(&_sz), interproc::BLOCK_DESCRIPTOR_SIZE);
    }

    inline void read_size(ibufstream& _s, block_descriptor_t &_sz){
        byte_t* lb = reinterpret_cast<byte_t*>(&_sz);
        _s.read(lb, interproc::BLOCK_DESCRIPTOR_SIZE);
    }
}

#endif //INTERPROCESS_BUFFER_HPP
