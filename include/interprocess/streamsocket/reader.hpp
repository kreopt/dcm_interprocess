#ifndef __INTERPROC_BUFFER_READER_
#define __INTERPROC_BUFFER_READER_

#include <functional>
#include <asio/error_code.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/streambuf.hpp>
#include <asio/buffers_iterator.hpp>
#include "../core/buffer.hpp"

namespace interproc {
    namespace streamsocket {
        // TODO: timeouts
        template<typename socket_type, typename buffer_type = interproc::buffer>
        class reader {
            using handler_t = std::function<void(const asio::error_code &_error, const long unsigned int&)>;
            using socket_ptr = std::shared_ptr<socket_type>;
        protected:
            buffer_type         buffer_;
            asio::streambuf     response_;
            std::atomic_bool    streambuf_read_;
            socket_ptr          socket_;
            handler_t           default_handler_;
        public:

            explicit reader(socket_ptr _socket) : socket_(_socket) {
                default_handler_ = std::bind(&interproc::streamsocket::reader<socket_type>::handle_read, this, std::placeholders::_1);
            };

            void read(int _size = -1, handler_t _handler = nullptr) {
                buffer_.clear();
                if (_size >= 0) {
                    streambuf_read_ = false;
                    buffer_.resize(_size);
                    if (_size) {
                        const char *bufstart = buffer_.data();
                        asio::async_read(*socket_, asio::buffer(const_cast<char*>(bufstart), _size), (_handler ? _handler : default_handler_));
                    } else {
                        handle_read(asio::error_code());
                    }
                } else {
                    streambuf_read_ = true;
                    asio::async_read(*socket_, response_,
                            asio::transfer_at_least(1),
                            (_handler ? _handler : default_handler_));
                }
            }

            void handle_read(const asio::error_code &_error) {
                if (!_error) {
                    if (on_success) {
                        buffer_type data;
                        if (streambuf_read_) {
                            buffer_.clear();
                            asio::streambuf::const_buffers_type bufs = response_.data();
                            data = buffer_type(asio::buffer_cast<const char*>(bufs), response_.size());
                            response_.consume(response_.size());
                        } else {
                            data = buffer_type(buffer_.data(), buffer_.size());
                        }
                        on_success(std::move(data));
                    }
                } else {
                    if (on_fail) on_fail(_error);
                }
            }

            std::function<void(const asio::error_code &)> on_fail;
            std::function<void(buffer_type &&_buffer)> on_success;
        };
    }
}

#endif