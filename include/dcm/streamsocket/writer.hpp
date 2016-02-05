#ifndef __INTERPROC_BUFFER_WRITER_
#define __INTERPROC_BUFFER_WRITER_

#include <functional>
#include <memory>
#include <thread>
#include <condition_variable>
#include <deque>
#include <asio.hpp>
#include <chrono>
#include "../core/buffer.hpp"
#include "socket.hpp"

namespace dcm  {
    namespace streamsocket {
        using namespace std::string_literals;
        // TODO: timeouts
        template<typename socket_type, typename buffer_type = dcm::buffer>
        class writer {

            class buffer_info {
                char size[BLOCK_DESCRIPTOR_SIZE];
                buffer_type buffer;

            public:
                inline std::vector<asio::const_buffer> buffers() {
                    return {asio::const_buffer(&size, BLOCK_DESCRIPTOR_SIZE), asio::const_buffer(buffer.data(), buffer.size())};
                };
                buffer_info() = delete;
                buffer_info(buffer_type &&_buf, block_descriptor_t _size) : buffer(_buf) {
                    memcpy(&size, reinterpret_cast<char*>(&_size), BLOCK_DESCRIPTOR_SIZE);
                }
                ~buffer_info(){
                }
            };

            std::deque<buffer_info>                      buffer_queue_;
            std::unique_ptr<std::thread>                 queue_thread_;
            std::condition_variable                      queue_cv_;
            std::condition_variable                      writer_cv_;
            std::mutex                                   queue_mutex_;
            bool                                         notified_;
            bool                                         notified_w_;
            std::atomic_bool                             stopped_;
            std::atomic_bool                             wait_send_;

            // Event handlers
            void handle_write(const asio::error_code &error, std::size_t bytes_transferred) {
                notified_w_ = true;
                writer_cv_.notify_all();

                if (!error) {
                    if (on_success) on_success();
                }
                else {
                    if (on_fail) on_fail(error);
                }

            }


        protected:
            std::shared_ptr<socket<socket_type>> socket_;

        public:
            explicit writer(std::shared_ptr<socket<socket_type>> _socket) : socket_(_socket) {
                notified_w_ = false;
                notified_ = false;
                stopped_ = true;
                wait_send_ = false;
                buffer_queue_.clear();
            }

            ~writer() {
                stop();
                wait_until_stopped();
            }

            void sender_thread_func(){
                while (!stopped_) {
                    {
                        std::unique_lock<std::mutex> lck(queue_mutex_);
                        if (!buffer_queue_.size()) {
                            queue_cv_.wait(lck, [this]() { return notified_; });
                        }
                        if (stopped_) {
                            return;
                        }

                        notified_=false;
                        if (socket_ && socket_->is_open()) {
                            asio::async_write(*socket_->get_socket(),
                                              buffer_queue_.front().buffers(),
                                              std::bind(&writer<socket_type, buffer_type>::handle_write,
                                                        this,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2));

                            writer_cv_.wait(lck, [this]() { return notified_w_; });
                            notified_w_ = false;
                            if (buffer_queue_.size()) {
                                buffer_queue_.pop_front();
                            }
                        }

                    }
                }
                if (wait_send_) {
                    std::unique_lock<std::mutex> lck(queue_mutex_);
                    while (buffer_queue_.size()) {
                        if (socket_ && socket_->is_open()) {
                            asio::async_write(*socket_->get_socket(),
                                              buffer_queue_.front().buffers(),
                                              std::bind(&writer<socket_type, buffer_type>::handle_write,
                                                        this,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2));

                            writer_cv_.wait(lck, [this]() { return notified_w_; });
                            notified_w_ = false;
                            if (buffer_queue_.size()) {
                                buffer_queue_.pop_front();
                            }
                        } else {
                            buffer_queue_.clear();
                        }
                    }
                }
            }


            void start() {
                notified_w_ = false;
                notified_ = false;
                stopped_ = false;
                buffer_queue_.clear();
                queue_thread_ = std::make_unique<std::thread>(std::bind(&writer<socket_type, buffer_type>::sender_thread_func, this));
            }

            void stop(bool wait_send = false) {
                if (wait_send) {
                    wait_send_ = true;
                }
                stopped_ = true;
                if (!wait_send) {
                    if (socket_ && socket_->is_open()) {
                        socket_->close();
                    }
                    notified_ = true;
                    notified_w_=true;
                    queue_cv_.notify_all();
                    writer_cv_.notify_all();
                    buffer_queue_.clear();
                }
            }

            void wait_until_stopped() {
                if (queue_thread_ && queue_thread_->joinable()) {
                    queue_thread_->join();
                }
            }

            void write(buffer_type &&_buf) {
                if (!stopped_) {
                    std::lock_guard<std::mutex> lck(queue_mutex_);
                    buffer_queue_.emplace_back(std::forward<buffer_type>(_buf), static_cast<block_descriptor_t>(_buf.size()));
                    notified_ = true;
                    queue_cv_.notify_one();
                }
            }

            std::function<void(const asio::error_code &)> on_fail;
            std::function<void()> on_success;
        };
    }
}

#endif