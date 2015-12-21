#ifndef INTERPROCESS_IPC_LISTENER_HPP
#define INTERPROCESS_IPC_LISTENER_HPP

#include <atomic>
#include <thread>
#include <memory>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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
                        auto pt = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(1000);
                        if (mq_->timed_receive(&data, MQ_SIZE, recvd_size, priority, pt)) {
                            Log::d("message received");
                            std::string uuid = std::string(reinterpret_cast<char *>(data), recvd_size);

                            Log::d(uuid);

                            shared_memory_object shm (open_only, uuid.c_str(), read_write);
                            mapped_region region(shm, read_write);

                            char *mem = static_cast<char*>(region.get_address());
                            buffer_type buf(reinterpret_cast<char*>(mem+BLOCK_DESCRIPTOR_SIZE), region.get_size()-BLOCK_DESCRIPTOR_SIZE);
                            block_descriptor_t recv_cnt = reinterpret_cast<block_descriptor_t*>(mem)[0];
                            shared_memory_object::remove(uuid.c_str());

                            Log::d(std::string("receiver count:")+std::to_string(recv_cnt));
                            Log::d(std::string(reinterpret_cast<const char*>(buf.data()), buf.size()));
                        }
                    }
                });
            };
            virtual void stop() {
                Log::d("stop listener");
                stopped_ = true;
            };
            virtual void wait_until_stopped() {
                if (listener_thread_->joinable()) {
                    listener_thread_->join();
                }
                if (mq_) {
                    message_queue::remove(ep_.c_str());
                }
                mq_.reset();
            };
        };

        template <typename buffer_type>
        using ipc_listener = listener_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_LISTENER_HPP
