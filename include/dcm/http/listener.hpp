#ifndef INTERPROCESS_HTTP_LISTENER_HPP
#define INTERPROCESS_HTTP_LISTENER_HPP

#include <atomic>
#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../core/listener.hpp"
#include "../core/processing_queue.hpp"

namespace dcm  {
    namespace http {
        namespace {
            template<typename buffer_type = dcm::buffer>
            class listener_impl : public dcm::listener<buffer_type> {

            public:
                using session_ptr = typename session<buffer_type>::ptr;

                virtual ~listener_impl() {
                    stop();
                    wait_until_stopped();
                }

                explicit listener_impl(const std::string &_ep) {
                }

                virtual bool is_running() const { return false; };
                virtual std::string get_endpoint() const override { return ""; }

                virtual void start() override {
                };

                virtual void stop() override {
                };

                virtual void wait_until_stopped() override {
                };
            };
        }

        template<typename buffer_type>
        using http_listener = listener_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_LISTENER_HPP
