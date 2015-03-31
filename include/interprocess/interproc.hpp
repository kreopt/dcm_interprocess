#ifndef _DCM_INTERPROC_HPP_
#define _DCM_INTERPROC_HPP_
#include <string>
#include <sstream>
#include <type_traits>
#include <functional>
#include <memory>
#include <asio/error_code.hpp>

namespace interproc {
    using block_size_t = uint32_t;
    using byte_t = char;
    using buffer = std::basic_string<byte_t>;
    using ibufstream = std::basic_istringstream<byte_t>;
    using obufstream = std::basic_ostringstream<byte_t>;

    const uint32_t BLOCK_SIZE_SIZE = sizeof(block_size_t);

    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    inline buffer to_buffer(T v){
        return buffer(reinterpret_cast<byte_t*>(v), sizeof(T));
    }

    template <>
    inline buffer to_buffer(bool v){
        return buffer(static_cast<byte_t>(v?0:1), 1);
    }

    inline void write_size(obufstream& _s, block_size_t _sz){
        _s.write(reinterpret_cast<interproc::byte_t*>(&_sz), interproc::BLOCK_SIZE_SIZE);
    }

    inline void read_size(ibufstream& _s, block_size_t &_sz){
        char * lb = reinterpret_cast<char*>(&_sz);
        _s.read(lb, interproc::BLOCK_SIZE_SIZE);
    }

    template <typename buffer_type = interproc::buffer >
    class session{
    public:
        virtual ~session() {};
        virtual void send(const buffer_type &_buf) = 0;
    };

    template <typename buffer_type = interproc::buffer >
    class receiver {
    public:
        typedef typename std::remove_reference<buffer_type>::type buf_type;

        virtual ~receiver() {}

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void join() = 0;

        virtual void broadcast(const buffer_type &_buf) = 0;
        // TODO: send to single instance

        std::function<void(buffer_type &&_buf)> on_message;
        std::function<void(std::shared_ptr<session<buffer_type>>)> on_connect;
    };

    template <typename buffer_type = interproc::buffer >
    class sender {
    public:
        virtual ~sender() {}

        virtual void connect() = 0;

        virtual void send(const buffer_type &_buf) = 0;

        virtual void close(const asio::error_code &error = asio::error_code()) = 0;
    };
}
#endif //_DCM_INTERPROC_HPP_
