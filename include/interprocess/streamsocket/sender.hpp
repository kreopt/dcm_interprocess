#ifndef __INTERPROC_SOCKET_CLIENT_
#define __INTERPROC_SOCKET_CLIENT_

#include <asio.hpp>
#include <thread>
#include <error.h>
#include <mutex>
#include <iostream>

#include "../interproc.hpp"
#include "../connection.hpp"
#include "endpoint.hpp"
#include "reader.hpp"
#include "writer.hpp"

using asio::ip::tcp;

namespace interproc {
    namespace streamsocket {

        template<typename protocol_type>
        class sender_impl : public sender<>,
                            public std::enable_shared_from_this<sender_impl<protocol_type>> {
        private:
            using socket_type = typename protocol_type::socket;
            using endpoint_type = typename protocol_type::endpoint;

            std::shared_ptr<asio::io_service> io_service_;
            std::shared_ptr<socket_type> socket_;
            typename protocol_type::endpoint endpoint_;
            std::thread client_thread_;
            volatile size_t pending_count_;
            std::mutex pending_ops_mtx_;
            std::mutex can_close_mtx_;
            std::shared_ptr<reader<socket_type>> reader_;
            std::shared_ptr<writer<socket_type>> writer_;
            volatile bool stopped_;
            volatile bool connected_;

            // Event handlers
            void handle_connect(const asio::error_code &error) {
                if (!error) {
                    std::cerr << "DCM sender connected" << std::endl;
                    socket_->set_option(typename socket_type::enable_connection_aborted(true));
                    socket_->set_option(typename socket_type::linger(true, 30));
                    connected_ = true;
                    this->reader_->read();
                } else {
                    //std::cerr << "DCM Client failed to connect: " << error.message() << std::endl;
                    connected_ = false;
                }
            }

        public:
            // Constructor
            explicit sender_impl(const std::string &_endpoint) {
                pending_count_ = 0;
                stopped_ = true;
                connected_ = false;
                io_service_ = std::make_shared<asio::io_service>();
                endpoint_ = interproc::make_endpoint<endpoint_type>(_endpoint, *io_service_);
                socket_ = std::make_shared<socket_type>(*io_service_);
                reader_ = std::make_shared<reader<socket_type>>(socket_);
                writer_ = std::make_shared<writer<socket_type>>(socket_);


                this->writer_->on_fail = this->reader_->on_fail = [this](const asio::error_code &error) {
                    if (error) {
                        std::cerr << error.message() << std::endl;
                    }
                    std::lock_guard<std::mutex> lck(pending_ops_mtx_);
                    can_close_mtx_.unlock();
                    pending_count_ = 0;
                    //close();    // TODO: try to reconnect
                    io_service_->stop();
                };;
                this->writer_->on_success = [this]() {
                    std::lock_guard<std::mutex> lck(pending_ops_mtx_);
                    pending_count_--;
                    if (pending_count_ <= 0) {
                        can_close_mtx_.unlock();
                    } else {
                        can_close_mtx_.try_lock();
                    }
                };
            }

            ~sender_impl() {
                std::cout << "destroy sender" << std::endl;
                can_close_mtx_.unlock();
                close();
            }

            virtual void connect() override {
                stopped_ = false;
                client_thread_ = std::thread([this]() {
                    while (!stopped_) {
                        if (socket_->is_open()) {
                            socket_->cancel();
                            socket_->close();
                        }
                        io_service_->stop();
                        io_service_->reset();

                        socket_->async_connect(endpoint_, std::bind(&sender_impl<protocol_type>::handle_connect, this->shared_from_this(), std::placeholders::_1));
                        io_service_->run();
                        can_close_mtx_.unlock();
                        connected_ = false;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                });
            }

            virtual void send(const buffer &_buf) override {
                if (connected_) {
                    std::cout << "try send" << std::endl;
                    {
                        std::lock_guard<std::mutex> lck(pending_ops_mtx_);
                        pending_count_++;
                    }
                    can_close_mtx_.try_lock();
                    this->writer_->write(_buf);
                } else {
                    std::cout << "failed to send: client disconnected" << std::endl;
                }
            }

            virtual void close(const asio::error_code &error = asio::error_code()) override {
                std::cerr << "close sender" << std::endl;
                stopped_ = true;
                can_close_mtx_.lock();
                if (socket_->is_open()) {
                    socket_->close();
                }
                if (!io_service_->stopped()) {
                    io_service_->stop();
                }
                if (error) {
                    std::cerr << error.message() << std::endl;
                }
            }
        };

        using tcp_sender = sender_impl<asio::ip::tcp>;
        using unix_sender = sender_impl<asio::local::stream_protocol>;

        inline std::shared_ptr<sender<>> make_sender(streamsocket_type _type, const std::string &_ep) {
            switch (_type) {
                case streamsocket_type::unix:
                    return std::make_shared<unix_sender>(_ep);
                case streamsocket_type::tcp:
                    return std::make_shared<tcp_sender>(_ep);
            }
        };
    }
}
#endif
