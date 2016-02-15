#ifndef INTERPROCESS_P2P_COMMUNICATOR_HPP
#define INTERPROCESS_P2P_COMMUNICATOR_HPP

#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include "../core/multicast.hpp"
#include "../listener_factory.hpp"
#include "event.hpp"
#include <binelpro/structure.hpp>
#include <memory>
namespace dcm  {

    template <bp::symbol::hash_type serializer_type, typename buffer_type = dcm::buffer>
    class p2p : public std::enable_shared_from_this<p2p<serializer_type, buffer_type>> {
    public:
        using ptr = std::shared_ptr<p2p<serializer_type>>;
        std::function<void(const bp::structure::ptr)> on_send_greeting;

    private:
        bool sender_stopped_;
        bool listener_stopped_;
        bool external_listener_;

        typename sender<buffer_type>::ptr    sender_;
        typename listener<buffer_type>::ptr  listener_;
        typename std::shared_ptr<dcm::event<serializer_type>>     event_;

        using event_handler_t = std::function<void(const bp::structure::ptr&)>;

        std::unordered_map<bp::symbol, event_handler_t> event_handlers;

        std::string listener_ep_;

        void send_greeting() {
            auto structure = bp::structure::create();
            structure->emplace("endpoint"_sym, listener_ep_);
            if (on_send_greeting) {
                on_send_greeting(structure);
            }
            send("p2p.greeting"_sym, std::move(structure));
        }
    public:
        p2p() : sender_stopped_(true), listener_stopped_(true), external_listener_(false) {}

        ~p2p() {
            disconnect();
            stop();
            wait_until_stopped();
        }

        void handle_message(const typename session<buffer_type>::ptr&, const bp::symbol &_evt, bp::structure::ptr&& _data) {
            if (event_handlers.count(_evt)) {
                event_handlers.at(_evt)(_data);
            }
        }

        p2p::ptr on(const bp::symbol &_evt, event_handler_t _handler) {
            event_handlers[_evt] = _handler;
            return this->shared_from_this();
        }

        void connect(const std::string &_endpoint, bool greet) {
            sender_ = dcm::make_sender(_endpoint);
            sender_->on_connect.set("p2p"_sym, [this, greet](){
                if (greet && !this->listener_ep_.empty()) {
                    this->send_greeting();
                }
            });
            sender_->connect();
        }
        void disconnect() {
            if (sender_) {
                sender_->close();
            }
            sender_.reset();
        }

        void start(const std::string &_endpoint) {
            external_listener_ = false;
            listener_ep_ = _endpoint;
            listener_ = make_listener(_endpoint);
            start(listener_, false);
        };

        void start(const typename dcm::listener<buffer_type>::ptr &_listener, bool _external = true) {
            listener_ep_ = _listener->get_endpoint();
            external_listener_ = _external;
            listener_ = _listener;
            event_ = std::make_shared<event<serializer_type, buffer_type>>(listener_);
            event_->on_event.set_default(std::bind(&p2p<serializer_type, buffer_type>::handle_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            listener_->start();
            if (sender_) {
                send_greeting();
            }
        }

        void stop() {
            if (sender_) {
                send("listener.stop"_sym, bp::structure::create());
            }
            listener_ep_="";
            if (external_listener_ && listener_) {
                listener_->stop();
            }
        }
        void wait_until_stopped() {
            if (listener_) {
                listener_->wait_until_stopped();
            }
        }

        void send(const dcm::buffer &_buf) {
            if (sender_) {
                sender_->send(_buf);
            }
        }

        void send(const bp::symbol &_evt, bp::structure::ptr &&_data = nullptr) {
            if (sender_) {
                sender_->send(make_event<serializer_type, buffer_type>(_evt, _data));
            }
        }

        static typename p2p<serializer_type, buffer_type>::ptr create() {
            return std::make_shared<p2p<serializer_type, buffer_type>>();
        }
    };
}

#endif //INTERPROCESS_P2P_COMMUNICATOR_HPP
