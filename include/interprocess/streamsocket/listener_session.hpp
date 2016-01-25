#ifndef __INTERPROC_RECEIVER_SESSION_HPP__
#define __INTERPROC_RECEIVER_SESSION_HPP__

#include <asio.hpp>
#include <error.h>

#include <memory>
#include <atomic>

#include "../core/listener.hpp"
#include "../core/processing_queue.hpp"
#include "reader.hpp"
#include "writer.hpp"

namespace interproc {
    namespace streamsocket {
        template<typename socket_type, typename buffer_type = interproc::buffer >
        class listener_session : public interproc::session<buffer_type>,
                                 public std::enable_shared_from_this<interproc::session<buffer_type>> {
        private:
            std::shared_ptr<socket_type>         socket_;
            interproc::processing_queue<buffer_type> handler_queue_;

        protected:
            std::atomic_bool                     eof_;
            std::atomic_bool                     started_;

            std::shared_ptr<reader<socket_type>> reader_;
            std::shared_ptr<writer<socket_type>> writer_;

            virtual void read() {
                // Check input buffer
                this->reader_->read();
            }
            virtual void read_success_handler(buffer_type &&_buffer) {
                handler_queue_.enqueue(std::forward<buffer_type>(_buffer));
                if (!eof_) {
                    this->reader_->read();
                }
            }
        public:

            // Constructor
            explicit listener_session(std::shared_ptr<asio::io_service> io_service)
                    : handler_queue_(1024*1024*1024), socket_(std::make_shared<socket_type>(*io_service)) {

                started_ = false;
                reader_ = std::make_shared<reader<socket_type>>(socket_);
                writer_ = std::make_shared<writer<socket_type>>(socket_);

                this->writer_->on_fail = this->reader_->on_fail = [this](const asio::error_code &error) {
                    if (error) {
                        Log::d(error.message());
                        eof_ = true;
                        // TODO: pass error to on_error function
                        if (on_error) on_error(this->shared_from_this());
                    }
                };

                eof_ = false;
                this->reader_->on_success = std::bind(&listener_session::read_success_handler, this, std::placeholders::_1);
            }

            virtual ~listener_session() {
                if (socket_->is_open()) {
                    socket_->close();
                }
                handler_queue_.stop();
                handler_queue_.wait_until_stopped();
                Log::d("destroy session");
            }

            virtual void send(const buffer_type &_buf) override {
                // TODO: only after start
                if (started_) {
                    this->writer_->write(_buf);
                }
            }

            // Session operations
            virtual void start() {
                read();
                started_ = true;
                handler_queue_.on_message = this->on_message;
                handler_queue_.start();
                if (on_connect) on_connect(this->shared_from_this());
            }

            // Overloads
            inline std::shared_ptr<socket_type> socket() const {
                return socket_;
            }

            interproc::msg_handler_t<buffer_type>                    on_message;
            std::function<void(typename session<buffer_type>::ptr _session)> on_error;
            interproc::connect_handler_t<buffer_type> on_connect;
        };
    }
}

#endif
