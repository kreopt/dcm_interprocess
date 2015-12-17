#ifndef INTERPROCESS_LISTENER_FACTORY_HPP
#define INTERPROCESS_LISTENER_FACTORY_HPP

#include "interprocess/core/util.hpp"
#include "connection.hpp"
#include "streamsocket/listener.hpp"
#include "ipc/listener.hpp"

namespace interproc {
    template <typename buffer_type = interproc::buffer>
    inline std::shared_ptr<listener<buffer_type>> make_listener(const std::string &_ep) {
        auto pos = _ep.find("://");
        if (pos==std::string::npos) {
            throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
        auto proto = _ep.substr(0, pos);
        auto ep = _ep.substr(pos+3);

        switch (protocol(symbol(proto))) {
            case protocol::unix:
                return std::make_shared<streamsocket::unix_listener<buffer_type>>(ep);
            case protocol::tcp:
                return std::make_shared<streamsocket::tcp_listener<buffer_type>>(ep);
            case protocol::ipc:
                return std::make_shared<ipc::ipc_listener<buffer_type>>(ep);
            default:
                throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
    };
}
#endif //INTERPROCESS_SENDER_FACTORY_HPP
