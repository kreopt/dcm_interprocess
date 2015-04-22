#ifndef INTERPROCESS_SENDER_HPP
#define INTERPROCESS_SENDER_HPP

#include "../interproc.hpp"

namespace interproc {
    namespace ipc {

        template <typename buffer_type = interproc::buffer >
        class sender_impl: public interproc::sender<buffer_type> {
        public:
            virtual ~sender_impl() {}

            explicit sender_impl(const std::string &_endpoint) {
            }

            virtual void connect() {

            };

            virtual void send(const buffer_type &_buf) {

            };

            virtual void close(){

            };
        };

        template <typename buffer_type>
        using ipc_sender = sender_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_SENDER_HPP
