#ifndef INTERPROCESS_IPC_SENDER_HPP
#define INTERPROCESS_IPC_SENDER_HPP

#include "../core/sender.hpp"
#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include <atomic>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


namespace interproc {
    namespace ipc {
        using namespace boost::interprocess;

        template <typename buffer_type = interproc::buffer >
        class sender_impl: public interproc::sender<buffer_type> {
            std::unique_ptr<message_queue>  mq_;
            std::string                     ep_;
            std::atomic_int                 msg_cnt_;
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
//                boost::uuids::random_generator gen;
//                boost::uuids::uuid uuid = gen();
//
//                std::string uid = boost::uuids::to_string(uuid);
                std::string uid(std::to_string(getmypid()).append(":").append(std::to_string(msg_cnt_)));
                sprintf(uid, "%d:%d", getmypid(), msg_cnt_);
                msg_cnt_++;


                shared_memory_object shm_obj(create_only, uid.c_str(), read_write);
                shm_obj.truncate(_buf.size()+BLOCK_DESCRIPTOR_SIZE);
                mapped_region region(shm_obj, read_write);

                // TODO: write receiver cnt
                std::memset(region.get_address(), BLOCK_DESCRIPTOR_SIZE, 0);
                std::memcpy(static_cast<byte_t*>(region.get_address())+BLOCK_DESCRIPTOR_SIZE, _buf.data(), _buf.size());

                buffer_type buf(uid, true);
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
