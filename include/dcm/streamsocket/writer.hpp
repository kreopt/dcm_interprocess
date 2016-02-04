#ifndef __INTERPROC_BUFFER_WRITER_
#define __INTERPROC_BUFFER_WRITER_

#include <functional>
#include <memory>
#include <asio.hpp>
#include "../core/buffer.hpp"

namespace dcm  {
    namespace streamsocket {
        using namespace std::string_literals;
        // TODO: timeouts
        template<typename socket_type, typename buffer_type = dcm::buffer>
        class writer {

            class buffer_info {
                char* size;
                buffer_type buffer;

            public:
                inline std::vector<asio::const_buffer> buffers() {
                    return {asio::buffer(size, BLOCK_DESCRIPTOR_SIZE), asio::buffer(buffer.data(), buffer.size())};
                };
                buffer_info(buffer_type &&_buf, size_t _size) : buffer(_buf){
                    size = new char[BLOCK_DESCRIPTOR_SIZE];
                    memcpy(size, reinterpret_cast<char*>(&_size), BLOCK_DESCRIPTOR_SIZE);
                }
                ~buffer_info(){
                    delete [] size;
                }
            };

            mutable std::deque<buffer_info>              buffer_queue_;
            std::unique_ptr<std::thread>                 queue_thread_;
            mutable std::condition_variable              queue_cv_;
            mutable std::condition_variable              writer_cv_;
            mutable std::mutex                           queue_mutex_;
            std::atomic_bool                             notified_;
            std::atomic_bool                             notified_w_;
            std::atomic_bool                             stopped_;
            std::atomic_bool                             wait_send_;

            // Event handlers
            void handle_write(const asio::error_code &error, std::size_t bytes_transferred) {
                notified_w_ = true;
                writer_cv_.notify_one();

                if (!error) {
                    if (on_success) on_success();
                }
                else {
                    if (on_fail) on_fail(error);
                }

            }


        protected:
            std::shared_ptr<socket_type> socket_;

        public:
            explicit writer(std::shared_ptr<socket_type> _socket) {
                socket_ = _socket;
                stopped_ = true;
                wait_send_ = false;
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
                            queue_cv_.wait(lck, [this]() { return static_cast<bool>(notified_); });
                        }
                        if (stopped_) {
                            return;
                        }
                        notified_=false;
                        asio::async_write(*socket_,
                                          buffer_queue_.front().buffers(),
                                          std::bind(&writer<socket_type, buffer_type>::handle_write,
                                                    this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));

                        writer_cv_.wait(lck, [this]() { return static_cast<bool>(notified_w_); });
                        notified_w_=false;
                        buffer_queue_.pop_front();
                    }
                }
                if (wait_send_) {
                    std::unique_lock<std::mutex> lck(queue_mutex_);
                    while (buffer_queue_.size()) {
                        asio::async_write(*socket_,
                                          buffer_queue_.front().buffers(),
                                          std::bind(&writer<socket_type, buffer_type>::handle_write,
                                                    this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));

                        writer_cv_.wait(lck, [this]() { return static_cast<bool>(notified_w_); });
                        notified_w_=false;
                        buffer_queue_.pop_front();
                    }
                }
            }



            void start() {
                stopped_ = false;
                queue_thread_ = std::make_unique<std::thread>(std::bind(&writer<socket_type, buffer_type>::sender_thread_func, this));
            }

            void stop(bool wait_send = false) {
                if (wait_send) {
                    wait_send_ = true;
                }
                stopped_ = true;
                if (!wait_send) {
                    queue_cv_.notify_all();
                }
            }

            void wait_until_stopped() {
                if (queue_thread_ && queue_thread_->joinable()) {
                    queue_thread_->join();
                }
            }

            void write(buffer_type &&_buf) {
                std::lock_guard<std::mutex> lck(queue_mutex_);
                buffer_queue_.emplace_back(std::forward<buffer_type>(_buf), _buf.size());
                notified_ = true;
                queue_cv_.notify_one();
            }

            std::function<void(const asio::error_code &)> on_fail;
            std::function<void()> on_success;
        };
    }
}

#endif