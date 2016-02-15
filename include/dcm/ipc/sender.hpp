#ifndef INTERPROCESS_IPC_SENDER_HPP
#define INTERPROCESS_IPC_SENDER_HPP

#include "dcm/core/sender.hpp"
#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include <binelpro/os.hpp>
#include <binelpro/log.hpp>
#include <atomic>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

namespace dcm  {
    namespace ipc {
        namespace {
            using bp::Log;
            using namespace boost::interprocess;

            // TODO: use unix socket or windows named pipe for message queue

            template<typename buffer_type = dcm::buffer>
            class endpoint_impl : public dcm::sender<buffer_type> {
                std::unique_ptr<message_queue> mq_;
                std::unique_ptr<named_mutex> queue_mutex_;
                std::string ep_;
                mutable std::atomic_bool connected_;
            public:
                virtual ~endpoint_impl() {
                    close();
                }

                explicit endpoint_impl(const std::string &_endpoint) : ep_(_endpoint), connected_(false) {
                }

                virtual bool connected() const { return connected_; };

                virtual void connect() {
                    Log::d("connecting");
                    try {
                        mq_ = std::make_unique<message_queue>(open_only, ep_.c_str());
                        queue_mutex_ = std::make_unique<named_mutex>(open_only, (ep_ + std::string(".mutex")).c_str());
                        connected_ = true;
                    } catch (...) {
                        throw std::runtime_error("failed to connect to message queue");
                    }
                };

                virtual void send(const buffer_type &_buf) const override {
                    send(buffer_type(_buf));
                };

                virtual void send(buffer_type &&_buf) const override {
                    if (!mq_) {
                        connected_ = false;
                        Log::d("message queue destroyed. reconnect");
                        throw std::runtime_error("message queue destroyed");
                    }

                    std::string id="1";
                    try {
                        shared_memory_object shm_obj(create_only, id.c_str(), read_write);
                        shm_obj.truncate(_buf.size() + BLOCK_DESCRIPTOR_SIZE);
                        mapped_region region(shm_obj, read_write);
                        // TODO: write receiver cnt
                        std::memset(region.get_address(), BLOCK_DESCRIPTOR_SIZE, 0);
                        std::memcpy(static_cast<byte_t *>(region.get_address()) + BLOCK_DESCRIPTOR_SIZE,
                                    _buf.data(),
                                    _buf.size());
                    } catch (boost::interprocess::interprocess_exception &_e) {
//                    throw std::runtime_error(std::string("failed to open shmem ").append(_buf.id).append(":\n")+_e.what());
                        // file already exists
                    }

                    buffer_type buf(id, true);

                    std::lock_guard<named_mutex> lock(*queue_mutex_);
                    if (!mq_ || !mq_->try_send(buf.data(), buf.size(), 0)) {
                        Log::d("failed to send");
//                    if (recv_cnt == 0) {
                        shared_memory_object::remove(id.c_str());
//                    }
                        throw std::runtime_error("failed to send message");
                    }
                };

                virtual void close(bool wait_queue=false) {
                    if (mq_) {
                        // TODO: wait queue
                        connected_ = false;
                        mq_.reset();
                    }
                };

                virtual std::string get_endpoint() const override {
                    return ep_;
                }
            };
        }

        template<typename buffer_type>
        using ipc_endpoint = endpoint_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_SENDER_HPP
