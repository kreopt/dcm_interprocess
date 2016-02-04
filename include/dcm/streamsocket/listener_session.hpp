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

namespace dcm  {
    namespace streamsocket {
        // TODO: only for buffers with .data() method

        template<typename socket_type, typename buffer_type = dcm::buffer >
        class listener_session : public dcm::session<buffer_type>,
                                 public std::enable_shared_from_this<dcm::session<buffer_type>> {
        private:
            std::shared_ptr<socket_type>         socket_;
            dcm::processing_queue<buffer_type>   handler_queue_;

        protected:
            std::atomic_bool                     eof_;
            std::atomic_bool                     started_;
            std::atomic_bool                     read_header_;

            std::shared_ptr<reader<socket_type>> reader_;
            std::shared_ptr<writer<socket_type>> writer_;

            virtual void read(int _size=-1) {
                // Check input buffer
                this->reader_->read(_size);
            }
            virtual void read_success_handler(buffer_type &&_buffer) {
                int size;
                if (read_header_) {
                    size = reinterpret_cast<block_descriptor_t*>(_buffer.data())[0];
                } else {
                    handler_queue_.enqueue(std::forward<buffer_type>(_buffer));
                    size = BLOCK_DESCRIPTOR_SIZE;
                }
                read_header_=!read_header_;
                if (!eof_) {
                    this->reader_->read(size);
                }
            }
        public:

            // Constructor
            explicit listener_session(std::shared_ptr<asio::io_service> io_service)
                    : handler_queue_(1024*1024*1024), socket_(std::make_shared<socket_type>(*io_service)) {

                started_ = false;
                reader_ = std::make_shared<reader<socket_type>>(socket_);
                writer_ = std::make_shared<writer<socket_type>>(socket_);

                this->writer_->on_fail = [this](const asio::error_code &error) {
                    if (error) {
                        Log::d("writer: "s + error.message());
                        eof_ = true;
                        // TODO: pass error to on_error function
                        if (on_error) on_error(this->shared_from_this());
                    }
                };
                this->reader_->on_fail = [this](const asio::error_code &error) {
                    if (error) {
                        Log::d("reader: "s + error.message());
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

            virtual void send(buffer_type &&_buf) override {
                // TODO: only after start
                if (started_) {
                    this->writer_->write(std::forward<buffer_type>(_buf));
                }
            }

            // Session operations
            virtual void start() {
                read_header_=true;
                read(BLOCK_DESCRIPTOR_SIZE);
                started_ = true;
                handler_queue_.on_message = this->on_message;
                handler_queue_.start();
                if (on_connect) on_connect(this->shared_from_this());
            }

            // Overloads
            inline std::shared_ptr<socket_type> socket() const {
                return socket_;
            }

            dcm::msg_handler_t<buffer_type>                    on_message;
            std::function<void(typename session<buffer_type>::ptr _session)> on_error;
            dcm::connect_handler_t<buffer_type> on_connect;
        };
    }
}

#endif
