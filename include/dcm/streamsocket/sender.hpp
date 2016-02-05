#ifndef __INTERPROC_SOCKET_CLIENT_
#define __INTERPROC_SOCKET_CLIENT_

#include <asio.hpp>
#include <thread>
#include <atomic>
#include <error.h>
#include <mutex>
#include <iostream>
#include <condition_variable>

#include "../core/buffer.hpp"
#include "../core/endpoint.hpp"
#include "asio_endpoint.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "socket.hpp"

using asio::ip::tcp;

namespace dcm  {
    namespace streamsocket {
        const size_t QUEUE_SIZE = 1024*1024*1024;       // TODO: make it configurable
        namespace {
            template<typename protocol_type, typename buffer_type = dcm::buffer>
            class endpoint_impl : public endpoint<buffer_type>,
                                  public std::enable_shared_from_this<endpoint_impl<protocol_type>> {
            public:
                using ptr = endpoint_impl<protocol_type, buffer_type>;
            private:
                using socket_type = typename protocol_type::socket;
                using endpoint_type = typename protocol_type::endpoint;

                std::shared_ptr<asio::io_service>   io_service_;
                std::shared_ptr<socket<socket_type>>        socket_;    // TODO: use lockable socket
                typename protocol_type::endpoint endpoint_;
                std::thread client_thread_;
                std::shared_ptr<typename dcm::streamsocket::reader<socket_type, buffer_type>> reader_;
                std::shared_ptr<typename dcm::streamsocket::writer<socket_type, buffer_type>> writer_;
                std::atomic_bool stopped_;
                std::atomic_bool connected_;
                std::atomic_bool was_connected_;
                std::promise<bool> connect_promise_;

                // Event handlers
                void handle_connect(const asio::error_code &error) {
                    if (!error) {
                        Log::d("DCM endpoint connected");
                        was_connected_ = true;
                        socket_->get_socket()->set_option(typename socket_type::enable_connection_aborted(true));
                        socket_->get_socket()->set_option(typename socket_type::linger(true, 30));
                        connected_ = true;
                        this->writer_->start();
                        if (this->on_connect) {
                            this->on_connect();
                        }
                        this->reader_->read();
                    } else {
                        //std::cerr << "DCM Client failed to connect: " << error.message() << std::endl;
                        connected_ = false;
                        io_service_->stop();
                        was_connected_ = false;
                    }

                    connect_promise_.set_value(static_cast<const bool>(connected_));
                }

            public:
                // Constructor
                explicit endpoint_impl(const std::string &_endpoint) {
                    was_connected_ = false;
                    stopped_ = true;
                    connected_ = false;
                    io_service_ = std::make_shared<asio::io_service>();
                    endpoint_ = dcm::streamsocket::make_endpoint<endpoint_type>(_endpoint, *io_service_);
                    socket_ = std::make_shared<socket<socket_type>>(io_service_);
                    reader_ = std::make_shared<reader<socket_type>>(socket_);
                    writer_ = std::make_shared<writer<socket_type>>(socket_);

                    this->writer_->on_fail = this->reader_->on_fail = [this](const asio::error_code &error) {
                        if (error) {
                            Log::e("writer fail "s + error.message());
                        }
                        connected_ = false;
                        io_service_->stop();
                    };
                }

                ~endpoint_impl() {
                    Log::d("destroy endpoint");
                    close();
                }

                virtual bool connected() const override { return connected_; }

                virtual std::future<bool> connect() override {
                    if (!stopped_) {
                        std::promise<bool> promise;
                        promise.set_value(true);
                        return promise.get_future();
                    }
                    stopped_ = false;

                    connect_promise_ = std::promise<bool>();

                    client_thread_ = std::thread([this]() {
                        while (!stopped_) {
                            if (socket_->is_open()) {
                                socket_->close();
                            }
                            if (!io_service_->stopped()) {
                                io_service_->stop();
                            }
                            io_service_->reset();

                            socket_->get_socket()->async_connect(endpoint_, std::bind(&endpoint_impl<protocol_type>::handle_connect,
                                                                        this->shared_from_this(),
                                                                        std::placeholders::_1));
                            io_service_->run();
                            connected_ = false;
                            this->writer_->stop();
                            this->writer_->wait_until_stopped();
                            connect_promise_ = std::promise<bool>();
                            auto fut = connect_promise_.get_future();
                            if (was_connected_) {
                                Log::d("disconnected");
                            }
                            if (this->on_disconnect && was_connected_) {
                                this->on_disconnect();
                                was_connected_ = false;
                            }
                            if (!stopped_) {
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                            }
                        }
                    });
                    return connect_promise_.get_future();
                }

                virtual void send(message<buffer_type> &&_buf) const override {
                    // TODO: sender queue. watch connection state (disconnected, connecting, connected)
                    if (connected_) {
                        this->writer_->write(std::move(_buf.data));
                    } else {
                        Log::e("failed to send: client disconnected");
                    }
                }

                virtual void close(bool wait_queue=false) override {
                    if (!stopped_) {
                        Log::d("close endpoint");

                        this->writer_->stop(wait_queue);
                        this->writer_->wait_until_stopped();

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
