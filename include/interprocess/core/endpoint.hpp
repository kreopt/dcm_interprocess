#ifndef INTERPROCESS_SENDER_HPP
#define INTERPROCESS_SENDER_HPP

#include <memory>
#include <binelpro/symbol.hpp>
#include <binelpro/os.hpp>
#include "buffer.hpp"
#include "processing_queue.hpp"

namespace interproc {

//    namespace {
        template<typename buffer_type = buffer>
        struct message {
            std::string id;
            buffer_type data;
        };
//    }

    template <typename buffer_type = interproc::buffer >
    class endpoint {
    public:
        using ptr = std::shared_ptr<endpoint<buffer_type>>;

        virtual ~endpoint() {}

        virtual bool connected() const = 0;

        virtual void connect() = 0;

        virtual void send(const message<buffer_type> &_buf) const = 0;

        virtual void close() = 0;


        // TODO: on_error function
        // TODO: on_disconnect function
    };

    const size_t QUEUE_SIZE = 1024*1024*1024;       // TODO: make it configurable
    template <typename buffer_type = interproc::buffer >
    class sender {
        struct queue_item {
            std::vector<typename endpoint<buffer_type>::ptr> endpoints;
            message<buffer_type>                             msg;

            size_t size() {
                return msg.data.size() + msg.id.size() + endpoints.size()*sizeof(typename endpoint<buffer_type>::ptr);
            }
        };

        std::atomic_ullong                      msg_cnt_;
        processing_queue<queue_item>            sender_queue_;

        std::string make_uid() {
            msg_cnt_++;
            return  std::to_string(bp::getmypid()).append(":").append(std::to_string(msg_cnt_));
        }

    public:
        sender() : msg_cnt_(0), sender_queue_(QUEUE_SIZE){
            sender_queue_.on_message = [](queue_item &&_msg){
                // send to ep list
                for (auto ep: _msg.endpoints) {
                    try {
                        if (!ep->connected()) {
                            ep->connect();
                        }
                        ep->send(_msg.msg);
                    } catch (std::runtime_error &_e) {
                        Log::w("failed to send");
                    }
                }
            };
        }
        ~sender() {
            close();
            sender_queue_.wait_until_stopped();
        }

        void connect() {
            sender_queue_.start();
        };
        void send(const std::vector<typename endpoint<buffer_type>::ptr> &_ep, buffer_type &&_buf) {
            sender_queue_.enqueue(queue_item{_ep, message<buffer_type>{make_uid(), std::move(_buf)}});
        };
        void send(const std::vector<typename endpoint<buffer_type>::ptr> &_ep, const buffer_type &_buf) {
            send(_ep, std::move(buffer_type(_buf)));
        };
        void close() {
            sender_queue_.stop();
        };
    };

    using default_sender = endpoint<>;
}

#endif //INTERPROCESS_SENDER_HPP