#ifndef INTERPROCESS_IPC_LISTENER_HPP
#define INTERPROCESS_IPC_LISTENER_HPP

#include <atomic>
#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../core/listener.hpp"
#include "../core/processing_queue.hpp"

namespace interproc {
    namespace ipc {
        const size_t QUEUE_SIZE = 1024*1024*1024;

        using namespace boost::interprocess;
        // TODO: use unix socket or windows named pipe for message queue

        template <typename buffer_type = interproc::buffer >
        class listener_impl : public interproc::listener<buffer_type>{
            std::unique_ptr<message_queue>       mq_;
            named_mutex                          queue_mutex_;
            std::unique_ptr<std::thread>         listener_thread_;
            std::string                          ep_;
            std::atomic_bool                     stopped_;
            interproc::processing_queue<buffer_type>        handler_queue_;

            std::string read_uid() {
                uint32_t    priority;
                message_queue::size_type recvd_size;
                byte_t      data[MQ_SIZE];
                auto pt = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(1000);
                if (mq_->timed_receive(&data, MQ_SIZE, recvd_size, priority, pt)) {
                    return std::string(reinterpret_cast<char *>(data), recvd_size);
                } else {
                    return "";
                }
            }
        public:
            using session_ptr = typename session<buffer_type>::ptr;

            virtual ~listener_impl() {
                stop();
                wait_until_stopped();
            }
            explicit listener_impl(const std::string &_ep) : handler_queue_(QUEUE_SIZE), ep_(_ep),
                                                             queue_mutex_(open_or_create, (_ep+std::string(".mutex")).c_str()){
                // TODO: throw or return invalid state?
                message_queue::remove(ep_.c_str());
                mq_ = std::make_unique<message_queue>(create_only, _ep.c_str(), MQ_AMOUNT, MQ_SIZE);
            }

            virtual void start() override {
                Log::d("start listener");
                stopped_ = false;
                listener_thread_ = std::make_unique<std::thread>([this](){
                    while (!stopped_) {
                        std::string uuid = read_uid();
                        if (!uuid.empty()) {
                            Log::d(uuid);

                            shared_memory_object shm (open_only, uuid.c_str(), read_write);
                            mapped_region region(shm, read_write);

                            char *mem = static_cast<char*>(region.get_address());
                            buffer_type buf(reinterpret_cast<char*>(mem+BLOCK_DESCRIPTOR_SIZE), region.get_size()-BLOCK_DESCRIPTOR_SIZE);
                            block_descriptor_t recv_cnt = reinterpret_cast<block_descriptor_t*>(mem)[0];
                            shared_memory_object::remove(uuid.c_str());

                            Log::d(std::string("receiver count:") + std::to_string(recv_cnt));
                            if (this->on_message) {
                                handler_queue_.enqueue(std::move(buf));
                            }
                        }
                    }
                });
                handler_queue_.on_message = [this](buffer_type &&_buf){
                    if (this->on_message) {
                        this->on_message(std::forward<buffer_type>(_buf));
                    }
                };
                handler_queue_.start();
            };
            virtual void stop() override {
                Log::d("stop listener");
                stopped_ = true;
                handler_queue_.stop();
            };
            virtual void wait_until_stopped() override {
                if (listener_thread_ && listener_thread_->joinable()) {
                    listener_thread_->join();
                }
                handler_queue_.wait_until_stopped();
                if (mq_) {
                    std::lock_guard<named_mutex> lock(queue_mutex_);
                    while (true) {
                        std::string uid = read_uid();
                        std::cout << "remove " << uid << std::endl;
                        if (uid.empty()) break;
                        shared_memory_object::remove(uid.c_str());
                    }
                    message_queue::remove(ep_.c_str());
                }
                named_mutex::remove((ep_+std::string(".mutex")).c_str());
                mq_.reset();
            };
        };

        template <typename buffer_type>
        using ipc_listener = listener_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_LISTENER_HPP
