#ifndef INTERPROCESS_LISTENER_FACTORY_HPP
#define INTERPROCESS_LISTENER_FACTORY_HPP

#include "connection.hpp"
#include "streamsocket/listener.hpp"
#include "ipc/listener.hpp"

namespace interproc {
    template <typename buffer_type = interproc::buffer>
    inline std::shared_ptr<listener<buffer_type>> make_listener(conn_type _type, const std::string &_ep) {
        switch (_type) {
            case conn_type::unix:
                return std::make_shared<streamsocket::unix_listener<buffer_type> >(_ep);
            case conn_type::tcp:
                return std::make_shared<streamsocket::tcp_listener<buffer_type> >(_ep);
            case conn_type::ipc:
                return std::make_shared<ipc::ipc_listener<buffer_type>>(_ep);
            default:
                return nullptr;
        }
    };
}
#endif //INTERPROCESS_SENDER_FACTORY_HPP
