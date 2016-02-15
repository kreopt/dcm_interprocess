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
#include "dcm/core/sender.hpp"
#include "asio_endpoint.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "socket.hpp"

using asio::ip::tcp;

namespace dcm  {
    namespace streamsocket {
        namespace {
            template<typename protocol_type, typename buffer_type = dcm::buffer>
            class endpoint_impl : public sender<buffer_type>,
                                  public std::enable_shared_from_this<endpoint_impl<protocol_type>> {
            public:

                using ptr = endpoint_impl<protocol_type, buffer_type>;
            private:
                using socket_type = typename protocol_type::socket;
                using endpoint_type = typename protocol_type::endpoint;

                std::shared_ptr<asio::io_service>   io_service_;
                std::shared_ptr<socket<socket_type>>        socket_;
                typename protocol_type::endpoint endpoint_;
                std::string endpoint_str_;
                std::thread client_thread_;
                std::shared_ptr<typename dcm::streamsocket::reader<socket_type, buffer_type>> reader_;
                std::shared_ptr<typename dcm::streamsocket::writer<socket_type, buffer_type>> writer_;
                std::atomic_bool stopped_;
                std::atomic_bool connected_;
                std::atomic_bool was_connected_;

                // Event handlers
                void handle_connect(const asio::error_code &error) {
                    if (!error) {
                        Log::d("DCM sender connected");
                        was_connected_ = true;
                        socket_->get_socket()->set_option(typename socket_type::enable_connection_aborted(true));
                        socket_->get_socket()->set_option(typename socket_type::linger(true, 30));
                        connected_ = true;
                        this->writer_->start();
                        this->on_connect.call();
                        this->reader_->read();
                    } else {
                        //std::cerr << "DCM Client failed to connect: " << error.message() << std::endl;
                        connected_ = false;
                        io_service_->stop();
                        was_connected_ = false;
                    }
                }

            public:
                // Constructor
                explicit endpoint_impl(const std::string &_endpoint) : io_service_(std::make_shared<asio::io_service>()), endpoint_str_(_endpoint){
                    was_connected_ = false;
                    stopped_ = true;
                    connected_ = false;
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
                    Log::d("destroy sender");
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
                            if (was_connected_) {
                                Log::d("disconnected");
                            }
                            if (was_connected_) {
                                this->on_disconnect.call();
                                was_connected_ = false;
                            }
                            if (!stopped_) {
                                std::this_thread::sleep_for(std::chrono::seconds(1));
                            }
                        }
                    });
                }

                virtual void send(const buffer_type &_buf) const override {
                    send(buffer_type(_buf));
                }
                virtual void send(buffer_type &&_buf) const override {
                    // TODO: multicast queue. watch connection state (disconnected, connecting, connected)
                    if (connected_) {
                        this->writer_->write(std::move(_buf));
                    } else {
                        Log::e("failed to send: client disconnected");
                    }
                }

                virtual void close(bool wait_queue=false) override {
                    if (!stopped_) {
                        Log::d("close sender");
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

                virtual std::string get_endpoint() const override {
                    return endpoint_str_;
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
