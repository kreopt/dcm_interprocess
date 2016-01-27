#ifndef INTERPROCESS_IPC_SENDER_HPP
#define INTERPROCESS_IPC_SENDER_HPP

#include "../core/endpoint.hpp"
#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include <binelpro/os.hpp>
#include <binelpro/log.hpp>
#include <atomic>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>


namespace interproc {
    namespace ipc {
        using bp::Log;
        using namespace boost::interprocess;


        template <typename buffer_type = interproc::buffer >
        class sender_impl: public interproc::endpoint<buffer_type> {
            std::unique_ptr<message_queue>  mq_;
            std::unique_ptr<named_mutex>    queue_mutex_;
            std::string                     ep_;
            mutable std::atomic_bool        connected_;
        public:
            virtual ~sender_impl() {
                close();
            }

            explicit sender_impl(const std::string &_endpoint) : ep_(_endpoint), connected_(false) {
            }

            virtual bool connected() const {return connected_;};

            virtual void connect() {
                Log::d("connecting");
                try {
                    mq_ = std::make_unique<message_queue>(open_only, ep_.c_str());
                    queue_mutex_=std::make_unique<named_mutex>(open_only, (ep_+std::string(".mutex")).c_str());
                    connected_ = true;
                } catch (...) {
                    throw std::runtime_error("failed to connect to message queue");
                }
            };

            virtual void send(const message<buffer_type> &_buf) const override {
                if (!mq_) {
                    connected_ = false;
                    Log::d("message queue destroyed. reconnect");
                    throw std::runtime_error("message queue destroyed");
                }

                try {
                    shared_memory_object shm_obj(create_only, _buf.id.c_str(), read_write);
                    shm_obj.truncate(_buf.data.size() + BLOCK_DESCRIPTOR_SIZE);
                    mapped_region region(shm_obj, read_write);
                    // TODO: write receiver cnt
                    std::memset(region.get_address(), BLOCK_DESCRIPTOR_SIZE, 0);
                    std::memcpy(static_cast<byte_t*>(region.get_address())+BLOCK_DESCRIPTOR_SIZE, _buf.data.data(), _buf.data.size());
                } catch (boost::interprocess::interprocess_exception &_e) {
//                    throw std::runtime_error(std::string("failed to open shmem ").append(_buf.id).append(":\n")+_e.what());
                    // file already exists
                }

                buffer_type buf(_buf.id, true);

                std::lock_guard<named_mutex> lock(*queue_mutex_);
                if (!mq_ || !mq_->try_send(buf.data(), buf.size(), 0)){
                    Log::d("failed to send");
//                    if (recv_cnt == 0) {
                        shared_memory_object::remove(_buf.id.c_str());
//                    }
                    throw std::runtime_error("failed to send message");
                }
            };

            virtual void close(){
                if (mq_) {
                    connected_ = false;
                    mq_.reset();
                }
            };
        };

        template <typename buffer_type>
        using ipc_sender = sender_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_SENDER_HPP
