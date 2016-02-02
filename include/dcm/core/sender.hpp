//
// Created by kreopt on 30.01.16.
//

#ifndef INTERPROCESS_SENDER_HPP
#define INTERPROCESS_SENDER_HPP

#include "endpoint.hpp"
#include "../endpoint_factory.hpp"
#include "../core/defs.hpp"

namespace dcm  {

    class sender_error : public dcm::exception {
    public:
        sender_error(const char* _reason): dcm::exception(_reason) {}
    };

    const size_t QUEUE_SIZE = 1024*1024*1024;       // TODO: make it configurable
    template <typename buffer_type = dcm::buffer >
    class sender {
        struct queue_item {
            std::vector<typename endpoint<buffer_type>::ptr> endpoints;
            message<buffer_type>                             msg;

            size_t size() const {
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
        using ptr = std::shared_ptr<sender<buffer_type>>;

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
        void send(const typename endpoint<buffer_type>::ptr _ep, buffer_type &&_buf) {
            std::vector<typename endpoint<buffer_type>::ptr> v{_ep};
            send(v, std::forward<buffer_type>(_buf));
        };
        void send(const typename endpoint<buffer_type>::ptr _ep, const buffer_type &_buf) {
            std::vector<typename endpoint<buffer_type>::ptr> v{_ep};
            send(v, _buf);
        };
        void close() {
            sender_queue_.stop();
        };
    };

    template <typename buffer_type = dcm::buffer >
    class p2p_sender {
        sender<buffer_type>     sender_;
        typename endpoint<buffer_type>::ptr endpoint_;
    public:
        using ptr = std::shared_ptr<p2p_sender<buffer_type>>;
        p2p_sender(typename endpoint<buffer_type>::ptr _ep) : endpoint_(_ep) {}
        p2p_sender(const char* _ep) : endpoint_(make_endpoint(_ep)) {}

        void connect() {
            if (endpoint_) {
                endpoint_->connect();
            } else {
                throw sender_error("endpoint is not configured");
            }
            sender_.connect();
        };
        void send(const buffer_type &_buf) {
            if (endpoint_) {
                sender_.send(endpoint_, _buf);
            } else {
                throw sender_error("endpoint is not configured");
            }
        };
        void send(buffer_type &&_buf) {
            if (endpoint_) {
                sender_.send(endpoint_, std::forward<buffer_type>(_buf));
            } else {
                throw sender_error("endpoint is not configured");
            }
        };
        void close() {
            if (endpoint_) {
                endpoint_->close();
            }
            sender_.close();
        };
    };

    template <typename buffer_type = dcm::buffer>
    typename p2p_sender<buffer_type>::ptr make_p2p_sender(const std::string &_endpoint) {

        auto info = parse_endpoint(_endpoint);
        if (info.first == "ipc") {
            info.first = "p2pipc";
        }
        std::string p2p_endpoint = std::string(info.first).append("://").append(info.second);

        auto endpoint = make_endpoint(p2p_endpoint);

        return std::make_shared<p2p_sender<buffer_type>>(endpoint);
    }
}
#endif //INTERPROCESS_SENDER_HPP
