#ifndef INTERPROCESS_LISTENER_HPP
#define INTERPROCESS_LISTENER_HPP

#include <functional>
#include <memory>
#include <deque>
#include <condition_variable>
#include <atomic>
#include "buffer.hpp"

namespace interproc {

    template <typename buffer_type = interproc::buffer >
    using msg_handler_t = std::function<void(buffer_type&&)>;

    template <typename buffer_type = interproc::buffer >
    class session {
    public:
        using ptr = typename std::shared_ptr<session<buffer_type>>;

        virtual ~session() {};

        virtual void send(const buffer_type &_buf) = 0;
        virtual void start() = 0;
    };

    template <typename buffer_type = interproc::buffer >
    using connect_handler_t = std::function<void(typename session<buffer_type>::ptr)>;


    template <typename buffer_type = interproc::buffer >
    class listener {
    public:
        typedef typename std::remove_reference<buffer_type>::type buf_type;
        using ptr = typename std::shared_ptr<listener<buffer_type>>;

        virtual ~listener() {}

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void wait_until_stopped() = 0;

        connect_handler_t<buffer_type>                    on_connect;
        msg_handler_t<buffer_type>                        on_message;
    };

    template <typename buffer_type = interproc::buffer >
    class queue_based_buf_handler {
        std::unique_ptr<std::thread>         handler_thread_;
        std::deque<buffer_type>              buf_queue_;
        std::condition_variable              buf_queue_cv_;
        std::mutex                           buf_queue_mutex_;
        std::atomic_bool                     stopped_;
        bool                                 notified_;
        size_t                               queue_size_;
        std::atomic_ulong                    current_size_;

        void handle_thread_func() {
            while (!stopped_) {
                std::unique_lock<std::mutex> lck(buf_queue_mutex_);
                buf_queue_cv_.wait(lck, [this](){return notified_;});
                if (stopped_) {
                    return;
                }
                buffer_type buf;
                std::swap(buf_queue_.front(), buf);
                buf_queue_.pop_front();
                current_size_-=buf.size();
                this->on_message(std::move(buf));
                notified_=false;
            }
        }
    public:

        msg_handler_t<buffer_type>                        on_message;

        queue_based_buf_handler() = delete;
        queue_based_buf_handler(size_t _queue_size): queue_size_(_queue_size), current_size_(0) {
        }

        virtual void stop() {
            stopped_=true;
            notified_=true;
            buf_queue_cv_.notify_all();
        }
        virtual void wait_until_stopped() {
            if (handler_thread_->joinable()) {
                handler_thread_->join();
            }
        }
        virtual void start() {
            handler_thread_ = std::make_unique<std::thread>(std::bind(&queue_based_buf_handler<buffer_type>::handle_thread_func, this));
        }

        void enqueue(buffer_type &&_buf) {
            if (this->on_message) {
                Log::d("enq");
                // TODO: check for queue overflow
                if (current_size_+_buf.size() < queue_size_) {
                    std::unique_lock<std::mutex> lck(buf_queue_mutex_);
                    current_size_ += _buf.size();
                    buf_queue_.emplace_back(std::move(_buf));
                    notified_ = true;
                    buf_queue_cv_.notify_one();
                } else {
                    Log::w("queue is full. dropping buffer");
                }
            }
        }
    };
}
#endif //INTERPROCESS_LISTENER_HPP
