//
// Created by kreopt on 30.01.16.
//

#ifndef INTERPROCESS_SENDER_HPP
#define INTERPROCESS_SENDER_HPP

#include "sender.hpp"
#include "dcm/sender_factory.hpp"
#include "../core/defs.hpp"

namespace dcm  {

    template <typename buffer_type = dcm::buffer >
    class multicast {
        std::atomic_ullong                      msg_cnt_;
        std::string make_uid() {
            msg_cnt_++;
            return  std::to_string(bp::getmypid()).append(":").append(std::to_string(msg_cnt_));
        }
    public:
        using ptr = std::shared_ptr<multicast<buffer_type>>;

        multicast() : msg_cnt_(0){}
        ~multicast() {}

        void send(const std::vector<typename sender<buffer_type>::ptr> &_ep, buffer_type &&_buf) {
            for (auto &ep: _ep) {
                try {
//                    auto multicast_id = make_uid();
                    if (ep->connected()) {
                        // TODO: real multicast send
                        ep->send(_buf);
                    } else {
                        Log::w("Endpoint "s+ep->get_endpoint()+" is disconnected. Skip sending");
                    }
                } catch (std::runtime_error &_e) {
                    Log::w("failed to send: "s+_e.what());
                }
            }
        };

        handler<std::function<void(typename sender<buffer_type>::ptr)>>     on_connect;
        handler<std::function<void(typename sender<buffer_type>::ptr)>>     on_disconnect;
    };

}
#endif //INTERPROCESS_SENDER_HPP
