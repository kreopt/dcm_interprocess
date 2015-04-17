#ifndef INTERPROCESS_LISTENER_HPP
#define INTERPROCESS_LISTENER_HPP

#include <interprocess/interproc.hpp>

namespace interproc {
    namespace ipc {
        template <typename buffer_type = interproc::buffer >
        class listener_impl : public interproc::listener<buffer_type> {
        public:
            virtual ~listener_impl() {}

            explicit listener_impl(const std::string &_ep){}

            virtual void start() {};

            virtual void stop() {};

            virtual void wait_until_stopped() {};

            virtual void broadcast(const buffer_type &_buf) {};
            // TODO: send to single instance

            std::function<void(const buffer_type &_buf)> on_message;
            std::function<void(std::shared_ptr<session<buffer_type>>)> on_connect;
        };

        template <typename buffer_type>
        using ipc_listener = listener_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_LISTENER_HPP
