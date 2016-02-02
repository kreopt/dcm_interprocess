#ifndef INTERPROCESS_P2P_COMMUNICATOR_HPP
#define INTERPROCESS_P2P_COMMUNICATOR_HPP

#include "../core/defs.hpp"
#include "../core/sender.hpp"
#include "../listener_factory.hpp"
#include "event_listener.hpp"
#include <binelpro/structure.hpp>
#include <memory>
namespace dcm  {

    template <bp::symbol::hash_type serializer_type>
    class p2p : public event_listener<serializer_type> {
    public:
        using ptr = std::shared_ptr<p2p<serializer_type>>;
        std::function<void(const bp::structure::ptr)> on_send_greeting;

    private:
        bool sender_stopped_;
        bool listener_stopped_;

        typename p2p_sender<>::ptr  sender_;
        typename listener<>::ptr    listener_;

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
        p2p() : sender_stopped_(true), listener_stopped_(true) {}

        ~p2p() {
            disconnect();
            stop();
            wait_until_stopped();
        }

        void connect(const std::string &_endpoint, bool greet) {
            sender_ = dcm::make_p2p_sender(_endpoint);
            auto connected = sender_->connect().get();
            if (greet && connected && !listener_ep_.empty()) {
                send_greeting();
            }
        }
        void disconnect() {
            sender_->close();
            sender_.reset();
        }

        void start(const std::string &_endpoint) {
            listener_ep_ = _endpoint;
            listener_ = make_listener(_endpoint);
            listener_->on_message = std::bind(&p2p<serializer_type>::handle_message, this, std::placeholders::_1);
            listener_->start();
            if (sender_) {
                send_greeting();
            }
        };

        void start(const dcm::listener<dcm::buffer>::ptr _listener) {
            listener_ep_ = _listener->get_endpoint();
            listener_ = _listener;
            if (!listener_->is_running()) {
                listener_->start();
            }
            if (sender_) {
                send_greeting();
            }
        }

        void stop() {
            if (sender_) {
                send("listener.stop"_sym, bp::structure::create());
            }
            listener_ep_="";
            listener_->stop();
        }
        void wait_until_stopped() {
            listener_->wait_until_stopped();
        }

        void send(const bp::symbol &_evt, bp::structure::ptr &&_data) {
            if (sender_) {
                auto event = bp::structure::create();
                event->emplace("event"_sym, _evt);
                event->emplace("data"_sym, *_data);
                sender_->send(event->stringify<serializer_type>());
            }
        }

        static typename p2p<serializer_type>::ptr create() {
            return std::make_shared<p2p<serializer_type>>();
        }
    };
}

#endif //INTERPROCESS_P2P_COMMUNICATOR_HPP
