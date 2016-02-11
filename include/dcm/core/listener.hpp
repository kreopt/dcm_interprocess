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
#include "handler.hpp"

namespace dcm  {
    using bp::Log;

    template <typename buffer_type>
    class session;

    namespace {
        template <typename buffer_type>
        using session_handler_t = std::function<void(const typename session<buffer_type>::ptr&)>;
        template <typename buffer_type>
        using session_error_handler_t = std::function<void(const typename session<buffer_type>::ptr&, const std::error_code&)>;
        template <typename buffer_type>
        using session_msg_handler_t = std::function<void(const typename session<buffer_type>::ptr&, buffer_type &&)>;
    }

    template <typename buffer_type = dcm::buffer >
    class session : public std::enable_shared_from_this<session<buffer_type>> {
    public:
        using ptr = typename std::shared_ptr<session<buffer_type>>;

        virtual ~session() {};

        virtual void send(buffer_type &&_buf) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;

        handler<session_handler_t<buffer_type>>     on_connect;
        handler<session_error_handler_t<buffer_type>>     on_error;
        handler<session_msg_handler_t<buffer_type>> on_message;
    };

    template <typename buffer_type = dcm::buffer >
    class listener : public std::enable_shared_from_this<listener<buffer_type>>{
    public:
        using ptr = typename std::shared_ptr<listener<buffer_type>>;

        virtual ~listener() {}

        virtual std::string get_endpoint() const = 0;
        virtual bool is_running() const = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void wait_until_stopped() = 0;

        handler<session_handler_t<buffer_type>>     on_connect;
        handler<session_error_handler_t<buffer_type>>     on_error;
        handler<session_msg_handler_t<buffer_type>> on_message;

    };

    using default_listener = listener<>;
}
#endif //INTERPROCESS_LISTENER_HPP
