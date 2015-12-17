#ifndef _DCM_INTERPROC_HPP_
#define _DCM_INTERPROC_HPP_

#include <string>
#include <sstream>
#include <functional>
#include <memory>
#include "core/buffer.hpp"
namespace interproc {

    template <typename buffer_type = interproc::buffer >
    class session{
    public:
        virtual ~session() {};
        virtual void send(const buffer_type &_buf) = 0;
        virtual void start() = 0;
    };

    template <typename buffer_type = interproc::buffer >
    class listener {
    public:
        typedef typename std::remove_reference<buffer_type>::type buf_type;
        using ptr = std::shared_ptr<listener<buffer_type>>;

        virtual ~listener() {}

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void wait_until_stopped() = 0;

        virtual void broadcast(const buffer_type &_buf) = 0;
        // TODO: send to single instance

        std::function<void(const buffer_type &_buf)> on_message;
        std::function<void(std::shared_ptr<session<buffer_type>>)> on_connect;
    };

    template <typename buffer_type = interproc::buffer >
    class sender {
    public:
        using ptr = std::shared_ptr<sender<buffer_type>>;

        virtual ~sender() {}

        virtual void connect() = 0;

        virtual void send(const buffer_type &_buf) = 0;

        virtual void close() = 0;

        //TODO: on_error function
    };
}
#endif //_DCM_INTERPROC_HPP_
