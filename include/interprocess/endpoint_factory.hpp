#ifndef INTERPROCESS_SENDER_FACTORY_HPP
#define INTERPROCESS_SENDER_FACTORY_HPP

#include <binelpro/symbol.hpp>
#include "streamsocket/sender.hpp"
#include "ipc/sender.hpp"

namespace interproc {
    template <typename buffer_type = interproc::buffer>
    inline typename endpoint<buffer_type>::ptr make_endpoint(const std::string &_ep) {
        // TODO: remove unix type
        auto info = parse_endpoint(_ep);
        switch (protocol(bp::symbol(info.first).hash)) {
            case protocol::unix:
                return std::make_shared<streamsocket::unix_endpoint<buffer_type>>(info.second);
            case protocol::tcp:
                return std::make_shared<streamsocket::tcp_endpoint<buffer_type>>(info.second);
            case protocol::ipc:
                return std::make_shared<ipc::ipc_endpoint<buffer_type>>(info.second);
            default:
                throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
    };
}
#endif //INTERPROCESS_SENDER_FACTORY_HPP
