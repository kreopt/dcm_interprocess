#ifndef __INTERPROC_RECEIVER_SESSION_HPP__
#define __INTERPROC_RECEIVER_SESSION_HPP__

#include <asio.hpp>
#include <error.h>

#include <iostream>

#include "../interproc.hpp"
#include "reader.hpp"
#include "writer.hpp"
namespace interproc {
    namespace streamsocket {
        template<typename socket_type, typename buffer_type = interproc::buffer >
        class listener_session : public interproc::session<buffer_type>,
                                 public std::enable_shared_from_this<interproc::session<buffer_type>> {
        private:
            std::shared_ptr<socket_type> socket_;
        protected:
            bool eof_;
            bool started_;

            std::shared_ptr<reader<socket_type>> reader_;
            std::shared_ptr<writer<socket_type>> writer_;

            virtual void read() {
                // Check input buffer
                this->reader_->read();
            }
            virtual void read_success_handler(buffer_type &&_buffer) {
                if (this->on_message) this->on_message(_buffer);
                if (!eof_) {
                    this->reader_->read();
                }
            }
        public:

            // Constructor
            explicit listener_session(asio::io_service &io_service)
                    : socket_(std::make_shared<socket_type>(io_service)) {

                started_ = false;
                reader_ = std::make_shared<reader<socket_type>>(socket_);
                writer_ = std::make_shared<writer<socket_type>>(socket_);

                this->writer_->on_fail = this->reader_->on_fail = [this](const asio::error_code &error) {
                    if (error) {
                        std::cerr << error.message() << std::endl;
                        eof_ = true;
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
                std::cout << "destroy session" << std::endl;
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
                if (on_connect) on_connect(this->shared_from_this());
            }

            // Overloads
            std::shared_ptr<socket_type> socket() {
                return socket_;
            }

            std::function<void(const buffer_type & _buf)> on_message;
            std::function<void(std::shared_ptr<interproc::session<buffer_type>> _session)> on_error;
            std::function<void(std::shared_ptr<interproc::session<buffer_type>> _session)> on_connect;
        };
    }
}

#endif
