#ifndef INTERPROCESS_IPC_LISTENER_HPP
#define INTERPROCESS_IPC_LISTENER_HPP

#include <atomic>
#include <thread>
#include <memory>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "../core/listener.hpp"

namespace interproc {
    namespace ipc {
        using namespace boost::interprocess;

        template <typename buffer_type = interproc::buffer >
        class listener_impl : public interproc::listener<buffer_type> {
            std::unique_ptr<message_queue>       mq_;
            std::unique_ptr<std::thread>         listener_thread_;
            std::string                          ep_;
            std::atomic_bool                     stopped_;
        public:
            using session_ptr = typename session<buffer_type>::ptr;

            virtual ~listener_impl() {
                stop();
                wait_until_stopped();
            }
            explicit listener_impl(const std::string &_ep) : ep_(_ep){
                // TODO: throw or return invalid state?
                message_queue::remove(ep_.c_str());
                mq_ = std::make_unique<message_queue>(create_only, _ep.c_str(), MQ_AMOUNT, MQ_SIZE);
            }

            virtual void start() {
                Log::d("start listener");
                listener_thread_ = std::make_unique<std::thread>([this](){
                    while (!stopped_) {
                        uint32_t    priority;
                        message_queue::size_type recvd_size;
                        byte_t      data[MQ_SIZE];
                        mq_->receive(&data, MQ_SIZE, recvd_size, priority);
                        Log::d("message received");
                        Log::d(std::string(reinterpret_cast<char*>(data), recvd_size));
                    }
                });
            };
            virtual void stop() {
                Log::d("stop listener");
                if (mq_) {
                    message_queue::remove(ep_.c_str());
                }
                mq_.reset();
            };
            virtual void wait_until_stopped() {
                if (listener_thread_->joinable()) {
                    listener_thread_->join();
                }
            };
        };

        template <typename buffer_type>
        using ipc_listener = listener_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_LISTENER_HPP
