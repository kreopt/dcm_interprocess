#ifndef __INTERPROC_SOCKET_SERVER_
#define __INTERPROC_SOCKET_SERVER_

#include <list>
#include <set>
#include <asio.hpp>
#include <thread>

#include "../interproc.hpp"
#include "../connection.hpp"
#include "endpoint.hpp"
#include "receiver_session.hpp"

using asio::ip::tcp;

namespace interproc {
    namespace streamsocket {

        // tcp_server class
        template <typename protocol_type, template <typename, typename> class session_type_tpl = interproc::streamsocket::receiver_session>
        class receiver_impl : public receiver<> {
        private:

            using acceptor_type = typename protocol_type::acceptor;
            using socket_type = typename protocol_type::socket;
            using session_type = session_type_tpl<typename protocol_type::socket, typename interproc::buffer>;

            std::shared_ptr<asio::io_service> io_service_;
            std::shared_ptr<acceptor_type> acceptor_;
            std::set<std::shared_ptr<session_type>> sessions_;
            std::thread server_thread_;
            asio::signal_set signals_;

            // Event handlers
            void handle_accept(std::shared_ptr<session_type> session, const asio::error_code &error) {
                std::cout << "client connected" << std::endl;
                if (!error) {
                    session->start();
                }
                start_accept();
            }

            // Initiates an asynchronous accept operation to wait for a new connection
            void start_accept() {
                auto new_session = std::make_shared<session_type>(*io_service_);
                new_session->on_message = on_message;
                new_session->on_error = [this](std::shared_ptr<session_type> _session) {
                    sessions_.erase(_session);
                };
                new_session->on_connect = on_connect;
                sessions_.insert(new_session);
                //acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true));
                acceptor_->async_accept(*new_session->socket(),
                        std::bind(&receiver_impl<protocol_type, session_type_tpl>::handle_accept,
                                this, new_session, std::placeholders::_1));
            }

            void do_await_stop() {
                signals_.async_wait([this](std::error_code /*ec*/, int /*signo*/) {
                    // The receiver is stopped by cancelling all outstanding asynchronous
                    // operations. Once all operations have finished the io_service::run()
                    // call will exit.
                    acceptor_->close();
                    sessions_.clear();
                    stop();
                });
            }

        public:
            using endpoint_type = typename protocol_type::endpoint;

            // Constructor
            explicit receiver_impl(const std::string &_endpoint) :
                    io_service_(std::make_shared<asio::io_service>()),
                    signals_(*io_service_) {
                endpoint_type ep = interproc::make_endpoint<endpoint_type>(_endpoint, *io_service_);
                acceptor_ = std::make_shared<acceptor_type>(*io_service_, ep);
                signals_.add(SIGINT);
                signals_.add(SIGTERM);
#if defined(SIGQUIT)
                signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
                do_await_stop();
            }

            ~receiver_impl() {
                std::cout << "destroy receiver" << std::endl;
                stop();
            }

            virtual void broadcast(const interproc::buffer &_buf) {
                for (auto &session: sessions_) {
                    session->send(_buf);
                }
            }

            virtual void start() override {
                server_thread_ = std::thread([this]() {
                    start_accept();
                    io_service_->run();
                    std::cout << "io stopped" << std::endl;
                });
            }

            virtual void stop() override {
                if (!io_service_->stopped()) {
                    io_service_->stop();
                }
                if (server_thread_.joinable()) {
                    server_thread_.join();
                }
            };

            virtual void join() override {
                if (server_thread_.joinable()) {
                    server_thread_.join();
                }
            };
        };

        using tcp_receiver = receiver_impl<asio::ip::tcp, interproc::streamsocket::receiver_session>;
        using unix_receiver = receiver_impl<asio::local::stream_protocol, interproc::streamsocket::receiver_session>;

        inline std::shared_ptr<receiver<>> make_receiver(streamsocket_type _type, const std::string &_ep) {
            switch (_type) {
                case streamsocket_type::unix: {
                    std::remove(_ep.c_str());
                    return std::make_shared<unix_receiver>(_ep);
                }
                case streamsocket_type::tcp:
                    return std::make_shared<tcp_receiver>(_ep);
                default:
                    return nullptr;
            }
        };
    }
}
#endif