#ifndef INTERPROCESS_IPC_SENDER_HPP
#define INTERPROCESS_IPC_SENDER_HPP

#include "../core/sender.hpp"

namespace interproc {
    namespace ipc {

        template <typename buffer_type = interproc::buffer >
        class sender_impl: public interproc::sender<buffer_type> {
            std::unique_ptr<message_queue>  mq_;
            std::string                     ep_;
        public:
            virtual ~sender_impl() {}

            explicit sender_impl(const std::string &_endpoint) : ep_(_endpoint) {
                close();
            }

            virtual void connect() {
                Log::d("connecting");
                mq_ = std::make_unique<message_queue>(open_only, ep_.c_str());
            };

            virtual void send(const buffer_type &_buf) {
                Log::d("sending");
                buffer_type buf = to_buffer(std::string("shmem_id"));
                mq_->send(buf.data(), buf.size(), 0);
            };

            virtual void close(){
                if (mq_) {
                    message_queue::remove(ep_.c_str());
                }
                mq_.reset();
            };
        };

        template <typename buffer_type>
        using ipc_sender = sender_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_SENDER_HPP
