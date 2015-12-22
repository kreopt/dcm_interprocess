#ifndef __INTERPROC_SOCKET_SERVER_
#define __INTERPROC_SOCKET_SERVER_

#include <list>
#include <set>
#include <asio.hpp>
#include <thread>
#include <mutex>

#if USE_STD_FS && defined(__has_include) && __has_include("experimental/filesystem")
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace filesystem = boost::filesystem;
#endif

#include "../core/listener.hpp"
#include "endpoint.hpp"
#include "listener_session.hpp"


namespace interproc {
    namespace streamsocket {

        template<typename protocol_type,
                typename buffer_type = interproc::buffer,
                template<typename, typename> class session_type_tpl = interproc::streamsocket::listener_session>
        class listener_impl : public interproc::listener<buffer_type> {
        public:
            using ptr           = listener_impl<protocol_type, buffer_type, session_type_tpl>;
            using endpoint_type = typename protocol_type::endpoint;
        private:

            using acceptor_type = typename protocol_type::acceptor;
            using socket_type   = typename protocol_type::socket;
            using session_type  = session_type_tpl<typename protocol_type::socket, buffer_type>;

            using acceptor_ptr = std::shared_ptr<acceptor_type>;
            using session_ptr = typename session<buffer_type>::ptr;

            std::shared_ptr<asio::io_service>       io_service_;
            std::shared_ptr<acceptor_type>          acceptor_;
            std::set<session_ptr>                   sessions_;
            std::thread                             server_thread_;
            std::mutex                              session_mutex_;
            asio::signal_set                        signals_;

            // Event handlers
            void handle_accept(session_ptr session, const asio::error_code &error) {
                Log::d("client connected");
                if (!error) {
                    session->start();
                }
                start_accept();
            }

            // Initiates an asynchronous accept operation to wait for a new connection
            void start_accept() {
                auto new_session = std::make_shared<session_type>(io_service_);
                new_session->on_message = this->on_message;
                new_session->on_error = [this](typename session<buffer_type>::ptr _session) {
                    std::lock_guard<std::mutex> lck(session_mutex_);
                    sessions_.erase(_session);
                };
                new_session->on_connect = this->on_connect;
                {
                    std::lock_guard<std::mutex> lck(session_mutex_);
                    sessions_.insert(new_session);
                }
                prepare_accept(acceptor_);
                acceptor_->async_accept(*new_session->socket(),
                                        std::bind(
                                                &listener_impl<protocol_type, buffer_type, session_type_tpl>::handle_accept,
                                                this, new_session, std::placeholders::_1));
            }

            inline void prepare_accept(acceptor_ptr _acceptor) {
                if (std::is_same<protocol_type, asio::ip::tcp>()) {
                    _acceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
                }
            }

            inline void prepare_endpoint(const std::string &_ep) {
                if (std::is_same<protocol_type, asio::local::stream_protocol>()) {
                    filesystem::remove(_ep);
                }
            }


        public:

            // Constructor
            explicit listener_impl(const std::string &_endpoint) :
                    io_service_(std::make_shared<asio::io_service>()),
                    signals_(*io_service_) {
                prepare_endpoint(_endpoint);
                endpoint_type ep = interproc::make_endpoint<endpoint_type>(_endpoint, *io_service_);
                acceptor_ = std::make_shared<acceptor_type>(*io_service_, ep);
            }

            ~listener_impl() {
                Log::d("destroy receiver");
                stop();
                wait_until_stopped();
            }

            [[deprecated]]
            virtual void broadcast(const interproc::buffer &_buf) {
                for (auto &session: sessions_) {
                    session->send(_buf);
                }
            }

            virtual void start() override {
                server_thread_ = std::thread([this]() {
                    start_accept();
                    io_service_->run();
                    Log::d("io stopped");
                });
            }
            virtual void stop() override {
                if (!io_service_->stopped()) {
                    io_service_->stop();
                }
            };
            virtual void wait_until_stopped() override {
                if (server_thread_.joinable()) {
                    server_thread_.join();
                }
            };
        };

        template<typename buffer_type>
        using tcp_listener = listener_impl<asio::ip::tcp, buffer_type, interproc::streamsocket::listener_session>;

        template<typename buffer_type>
        using unix_listener = listener_impl<asio::local::stream_protocol, buffer_type, interproc::streamsocket::listener_session>;

    }
}
#endif