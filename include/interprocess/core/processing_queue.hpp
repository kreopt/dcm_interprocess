#ifndef INTERPROCESS_QUEUE_HPP
#define INTERPROCESS_QUEUE_HPP

#include "buffer.hpp"
#include <binelpro/log.hpp>
#include <thread>
#include <deque>
#include <condition_variable>
#include <atomic>

namespace interproc {
    using bp::Log;

    template <typename item_type = interproc::buffer >
    class processing_queue {
        std::unique_ptr<std::thread>         handler_thread_;
        std::deque<item_type>                queue_;
        std::condition_variable              queue_cv_;
        std::mutex                           queue_mutex_;
        std::atomic_bool                     stopped_;
        bool                                 notified_;
        size_t                               max_size_;
        std::atomic_ulong                    current_size_;

        void handle_thread_func() {
            while (!stopped_) {
                std::unique_lock<std::mutex> lck(queue_mutex_);
                queue_cv_.wait(lck, [this](){return notified_;});
                if (stopped_) {
                    return;
                }
                item_type buf;
                std::swap(queue_.front(), buf);
                queue_.pop_front();
                current_size_-=buf.size();
                this->on_message(std::move(buf));
                notified_=false;
            }
        }
    public:

        msg_handler_t<item_type>             on_message;

        processing_queue() = delete;
        processing_queue(size_t _queue_size): max_size_(_queue_size), current_size_(0) {
        }

        void stop() {
            stopped_=true;
            notified_=true;
            queue_cv_.notify_all();
        }
        void wait_until_stopped() {
            if (handler_thread_->joinable()) {
                handler_thread_->join();
            }
        }
        void start() {
            stopped_ = false;
            notified_ = false;
            handler_thread_ = std::make_unique<std::thread>(std::bind(&processing_queue<item_type>::handle_thread_func, this));
        }
        void enqueue(item_type &&_buf) {
            if (this->on_message) {
                // TODO: check for queue overflow
                if (current_size_+_buf.size() < max_size_) {
                    std::unique_lock<std::mutex> lck(queue_mutex_);
                    current_size_ += _buf.size();
                    queue_.emplace_back(std::move(_buf));
                    notified_ = true;
                    queue_cv_.notify_one();
                } else {
                    Log::w(std::string("queue is full. dropping buffer ").append(std::to_string(current_size_+_buf.size())).append(" ").append(std::to_string(max_size_)));
                }
            }
        }
    };
}
#endif //INTERPROCESS_QUEUE_HPP