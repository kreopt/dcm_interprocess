#ifndef INTERPROCESS_IPC_SENDER_HPP
#define INTERPROCESS_IPC_SENDER_HPP

#include "../core/sender.hpp"
#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include <binelpro/os.hpp>
#include <binelpro/log.hpp>
#include <atomic>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>


namespace interproc {
    namespace ipc {
        using bp::Log;
        using namespace boost::interprocess;

        template <typename buffer_type = interproc::buffer >
        class sender_impl: public interproc::sender<buffer_type> {
            std::unique_ptr<message_queue>  mq_;
            std::string                     ep_;
            std::atomic_ullong              msg_cnt_;
        public:
            virtual ~sender_impl() {
                close();
            }

            explicit sender_impl(const std::string &_endpoint) : ep_(_endpoint), msg_cnt_(0) {
//                close();
            }

            // TODO: multicast

            virtual void connect() {
                Log::d("connecting");
                try {
                    mq_ = std::make_unique<message_queue>(open_only, ep_.c_str());
                } catch (...) {
                    throw std::runtime_error("failed to connect to message queue");
                }
            };

            virtual void send(const buffer_type &_buf) {
                if (!mq_) {
                    Log::d("message queue destroyed. reconnect");
                    throw std::runtime_error("message queue destroyed");
                }

                std::string uid(std::to_string(bp::getmypid()).append(":").append(std::to_string(msg_cnt_)));
                msg_cnt_++;

                shared_memory_object shm_obj(create_only, uid.c_str(), read_write);
                shm_obj.truncate(_buf.size()+BLOCK_DESCRIPTOR_SIZE);
                mapped_region region(shm_obj, read_write);

                // TODO: write receiver cnt
                std::memset(region.get_address(), BLOCK_DESCRIPTOR_SIZE, 0);
                std::memcpy(static_cast<byte_t*>(region.get_address())+BLOCK_DESCRIPTOR_SIZE, _buf.data(), _buf.size());

                buffer_type buf(uid, true);

                if (!mq_->try_send(buf.data(), buf.size(), 0)){
                    Log::d("failed to send");
                    shared_memory_object::remove(uid.c_str());
                    throw std::runtime_error("failed to send message");
                }
            };

            virtual void close(){
                if (mq_) {
                    try {
                        message_queue::remove(ep_.c_str());
                    } catch(...) {}
                    mq_.reset();
                }
            };
        };

        template <typename buffer_type>
        using ipc_sender = sender_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_SENDER_HPP
