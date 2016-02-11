#ifndef INTERPROCESS_LISTENER_HPP
#define INTERPROCESS_LISTENER_HPP

#include <functional>
#include <unordered_map>
#include <memory>
#include <deque>
#include <condition_variable>
#include <atomic>
#include <binelpro/log.hpp>
#include "buffer.hpp"

namespace dcm  {
    using bp::Log;

    template <typename buffer_type = dcm::buffer >
    class session {
    public:
        using ptr = typename std::shared_ptr<session<buffer_type>>;

        virtual ~session() {};

        virtual void send(buffer_type &&_buf) = 0;
        virtual void start() = 0;
    };

    template <typename buffer_type = dcm::buffer >
    using connect_handler_t = std::function<void(typename session<buffer_type>::ptr)>;


    template <typename buffer_type = dcm::buffer >
    class listener {
        std::unordered_map<bp::symbol, msg_handler_t<buffer_type>> message_handlers_;

        void handle_message(buffer_type && _buf) {
            auto data = event_matcher(_buf);
        }
    public:
        typedef typename std::remove_reference<buffer_type>::type buf_type;
        using ptr = typename std::shared_ptr<listener<buffer_type>>;

        virtual ~listener() {}

        virtual std::string get_endpoint() const = 0;
        virtual bool is_running() const = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void wait_until_stopped() = 0;

        virtual void on(const bp::symbol &_evt, msg_handler_t<buffer_type> _handler) {
            message_handlers_.emplace(_evt, _handler);
        };
        virtual void off(const bp::symbol &_evt) {
            message_handlers_.erase(_evt);
        };

        connect_handler_t<buffer_type>                    on_connect;
        msg_handler_t<buffer_type>                        on_message;
        event_matcher_t<buffer_type>                        event_matcher;
    };


    using default_listener = listener<>;
}
#endif //INTERPROCESS_LISTENER_HPP
