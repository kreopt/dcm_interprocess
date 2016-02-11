#ifndef INTERPROCESS_LISTENER_FACTORY_HPP
#define INTERPROCESS_LISTENER_FACTORY_HPP

#include <binelpro/symbol.hpp>
#include "streamsocket/listener.hpp"
#include "ipc/listener.hpp"

namespace dcm  {
    template <typename buffer_type = dcm::buffer>
    inline std::shared_ptr<listener<buffer_type>> make_listener(const std::string &_ep) {
        auto info = parse_endpoint(_ep);

        switch (protocol(bp::symbol(info.first).hash)) {
            case protocol::p2pipc:
#ifndef WIN32
                return std::make_shared<streamsocket::unix_listener<buffer_type>>(info.second);
#else
                return std::make_shared<ipc::ipc_listener<buffer_type>>(info.second);
#endif
            case protocol::tcp:
                return std::make_shared<streamsocket::tcp_listener<buffer_type>>(info.second);
            case protocol::ipc:
                return std::make_shared<ipc::ipc_listener<buffer_type>>(info.second);
            default:
                throw std::runtime_error("invalid protocol. supported types are ipc, tcp and p2pipc");
        }
    };
}
#endif //INTERPROCESS_SENDER_FACTORY_HPP
