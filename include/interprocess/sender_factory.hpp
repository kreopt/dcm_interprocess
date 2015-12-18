#ifndef INTERPROCESS_SENDER_FACTORY_HPP
#define INTERPROCESS_SENDER_FACTORY_HPP

#include "interprocess/core/util.hpp"
#include "streamsocket/sender.hpp"
#include "ipc/sender.hpp"

namespace interproc {
    template <typename buffer_type = interproc::buffer>
    inline std::shared_ptr<sender<buffer_type>> make_sender(const std::string &_ep) {
        auto info = parse_endpoint(_ep);
        switch (protocol(symbol(info.first))) {
            case protocol::unix:
                return std::make_shared<streamsocket::unix_sender<buffer_type>>(info.second);
            case protocol::tcp:
                return std::make_shared<streamsocket::tcp_sender<buffer_type>>(info.second);
            case protocol::ipc:
                return std::make_shared<ipc::ipc_sender<buffer_type>>(info.second);
            default:
                throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
    };
}
#endif //INTERPROCESS_SENDER_FACTORY_HPP
