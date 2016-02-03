#ifndef __INTERPROC_BUFFER_WRITER_
#define __INTERPROC_BUFFER_WRITER_

#include <functional>
#include <memory>
#include <asio.hpp>
#include "../core/buffer.hpp"

namespace dcm  {
    namespace streamsocket {
        // TODO: timeouts
        template<typename socket_type, typename buffer_type = dcm::buffer>
        class writer {
        protected:
            std::shared_ptr<socket_type> socket_;

        public:
            explicit writer(std::shared_ptr<socket_type> _socket) {
                socket_ = _socket;
            }

            void write(const buffer_type &_buf) {
                std::vector<asio::const_buffer> buffers;
                block_descriptor_t size = _buf.size();
                buffers.push_back(asio::buffer(&size, BLOCK_DESCRIPTOR_SIZE));
                buffers.push_back(asio::buffer(_buf.data(), _buf.size()));

                asio::async_write(*socket_, buffers,
                        std::bind(&writer<socket_type>::handle_write, this, std::placeholders::_1, std::placeholders::_2));
            }

            // Event handlers
            void handle_write(const asio::error_code &error, std::size_t bytes_transferred) {
                if (!error) {
                    if (on_success) on_success();
                }
                else {
                    if (on_fail) on_fail(error);
                }
            }

            std::function<void(const asio::error_code &)> on_fail;
            std::function<void()> on_success;
        };
    }
}

#endif