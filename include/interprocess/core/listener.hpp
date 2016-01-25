#ifndef INTERPROCESS_LISTENER_HPP
#define INTERPROCESS_LISTENER_HPP

#include <functional>
#include <memory>
#include <deque>
#include <condition_variable>
#include <atomic>
#include <binelpro/log.hpp>
#include "buffer.hpp"

namespace interproc {
    using bp::Log;

    template <typename buffer_type = interproc::buffer >
    class session {
    public:
        using ptr = typename std::shared_ptr<session<buffer_type>>;

        virtual ~session() {};

        virtual void send(const buffer_type &_buf) = 0;
        virtual void start() = 0;
    };

    template <typename buffer_type = interproc::buffer >
    using connect_handler_t = std::function<void(typename session<buffer_type>::ptr)>;


    template <typename buffer_type = interproc::buffer >
    class listener {
    public:
        typedef typename std::remove_reference<buffer_type>::type buf_type;
        using ptr = typename std::shared_ptr<listener<buffer_type>>;

        virtual ~listener() {}

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void wait_until_stopped() = 0;

        connect_handler_t<buffer_type>                    on_connect;
        msg_handler_t<buffer_type>                        on_message;
    };


    using default_listener = listener<>;
}
#endif //INTERPROCESS_LISTENER_HPP
