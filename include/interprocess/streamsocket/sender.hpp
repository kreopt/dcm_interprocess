#ifndef __INTERPROC_SOCKET_CLIENT_
#define __INTERPROC_SOCKET_CLIENT_

#include <asio.hpp>
#include <thread>
#include <atomic>
#include <error.h>
#include <mutex>
#include <iostream>

#include "../core/buffer.hpp"
#include "../core/endpoint.hpp"
#include "asio_endpoint.hpp"
#include "reader.hpp"
#include "writer.hpp"

using asio::ip::tcp;

namespace interproc {
    namespace streamsocket {

        namespace {
            template<typename protocol_type, typename buffer_type = interproc::buffer>
            class endpoint_impl : public endpoint<buffer_type>,
                                  public std::enable_shared_from_this<endpoint_impl<protocol_type>> {
            public:
                using ptr = endpoint_impl<protocol_type, buffer_type>;
            private:
                using socket_type = typename protocol_type::socket;
                using endpoint_type = typename protocol_type::endpoint;

                std::shared_ptr<asio::io_service> io_service_;
                std::shared_ptr<socket_type> socket_;
                typename protocol_type::endpoint endpoint_;
                std::thread client_thread_;
                std::shared_ptr<streamsocket::reader<socket_type, buffer_type>> reader_;
                std::shared_ptr<streamsocket::writer<socket_type, buffer_type>> writer_;
                std::atomic_bool stopped_;
                std::atomic_bool connected_;

                // Event handlers
                void handle_connect(const asio::error_code &error) {
                    if (!error) {
                        std::cerr << "DCM endpoint connected" << std::endl;
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
                explicit endpoint_impl(const std::string &_endpoint) {
                    stopped_ = true;
                    connected_ = false;
                    io_service_ = std::make_shared<asio::io_service>();
                    endpoint_ = interproc::streamsocket::make_endpoint<endpoint_type>(_endpoint, *io_service_);
                    socket_ = std::make_shared<socket_type>(*io_service_);
                    reader_ = std::make_shared<reader<socket_type>>(socket_);
                    writer_ = std::make_shared<writer<socket_type>>(socket_);


                    this->writer_->on_fail = this->reader_->on_fail = [this](const asio::error_code &error) {
                        if (error) {
                            std::cerr << error.message() << std::endl;
                        }
                        io_service_->stop();
                    };;
                    this->writer_->on_success = [this]() {
                    };
                }

                ~endpoint_impl() {
                    std::cout << "destroy endpoint" << std::endl;
                    close();
                }

                virtual bool connected() const override { return connected_; }

                virtual void connect() override {
                    if (!stopped_) {
                        return;
                    }
                    stopped_ = false;
                    client_thread_ = std::thread([this]() {
                        while (!stopped_) {
                            if (socket_->is_open()) {
                                socket_->cancel();
                                socket_->close();
                            }
                            io_service_->stop();
                            io_service_->reset();

                            socket_->async_connect(endpoint_, std::bind(&endpoint_impl<protocol_type>::handle_connect,
                                                                        this->shared_from_this(),
                                                                        std::placeholders::_1));
                            io_service_->run();
                            connected_ = false;
                            //std::cout << "disconnected" << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                    });
                }

                virtual void send(const message<buffer_type> &_buf) const override {
                    if (connected_) {
                        std::cout << "try send" << std::endl;
                        this->writer_->write(_buf.data);
                    } else {
                        std::cout << "failed to send: client disconnected" << std::endl;
                    }
                }

                virtual void close() override {
                    std::cerr << "close endpoint" << std::endl;
                    // TODO: wait pending operations if necessary
                    stopped_ = true;
                    if (socket_->is_open()) {
                        socket_->close();
                    }
                    if (!io_service_->stopped()) {
                        io_service_->stop();
                    }
                    if (client_thread_.joinable()) {
                        client_thread_.join();
                    }
                }
            };
        }
        template <typename buffer_type>
        using tcp_endpoint = endpoint_impl<asio::ip::tcp, buffer_type>;

        template <typename buffer_type>
        using unix_endpoint = endpoint_impl<asio::local::stream_protocol, buffer_type>;
    }
}
#endif
