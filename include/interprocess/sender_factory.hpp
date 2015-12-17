#ifndef INTERPROCESS_SENDER_FACTORY_HPP
#define INTERPROCESS_SENDER_FACTORY_HPP

#include "connection.hpp"
#include "interprocess/core/util.hpp"
#include "streamsocket/sender.hpp"
#include "ipc/sender.hpp"

namespace interproc {
    template <typename buffer_type = interproc::buffer>
    inline std::shared_ptr<sender<buffer_type>> make_sender(const std::string &_ep) {
        auto pos = _ep.find("://");
        if (pos==std::string::npos) {
            throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
        auto proto = _ep.substr(0, pos);
        auto ep = _ep.substr(pos+3);

        switch (protocol(symbol(proto))) {
            case protocol::unix:
                return std::make_shared<streamsocket::unix_sender<buffer_type>>(ep);
            case protocol::tcp:
                return std::make_shared<streamsocket::tcp_sender<buffer_type>>(ep);
            case protocol::ipc:
                return std::make_shared<ipc::ipc_sender<buffer_type>>(ep);
            default:
                throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
    };
}
#endif //INTERPROCESS_SENDER_FACTORY_HPP
