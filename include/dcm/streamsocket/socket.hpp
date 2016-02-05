#ifndef DCM_SOCKET_HPP
#define DCM_SOCKET_HPP

#include <mutex>
#include <memory>
#include <asio.hpp>

namespace dcm {
    namespace streamsocket {

        template <typename socket_type>
        class socket {
            std::shared_ptr<socket_type>        socket_;
            std::mutex                          socket_mutex_;
        public:
            explicit socket(std::shared_ptr<asio::io_service> _io_service) : socket_(std::make_shared<socket_type>(*_io_service)) {}

            inline std::shared_ptr<socket_type> get_socket() const {return socket_;};
            void close(bool cancel=true){
                if (socket_->is_open()) {
                    std::lock_guard<std::mutex> lck(socket_mutex_);
                    if (cancel) socket_->cancel();
                    socket_->close();
                }
            }
            bool is_open() {
                std::lock_guard<std::mutex> lck(socket_mutex_);
                return socket_->is_open();
            }
        };
    }
}
#endif //DCM_SOCKET_HPP
