#ifndef INTERPROCESS_HTTP_SENDER_HPP
#define INTERPROCESS_HTTP_SENDER_HPP

#include "../core/endpoint.hpp"
#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include <binelpro/os.hpp>
#include <binelpro/log.hpp>
#include <atomic>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

namespace dcm  {
    namespace http {
        namespace {
            using bp::Log;

            template<typename buffer_type = dcm::buffer>
            class endpoint_impl : public dcm::endpoint<buffer_type> {
            public:
                virtual ~endpoint_impl() {
                    close();
                }

                explicit endpoint_impl(const std::string &_endpoint) : ep_(_endpoint), connected_(false) {
                }

                virtual bool connected() const { return connected_; };

                virtual std::future<bool> connect() {
                    Log::d("connecting");
                    std::promise<bool> promise;
                    try {
                        connected_ = true;
                        promise.set_value(true);
                    } catch (...) {
                        promise.set_value(false);
                        throw std::runtime_error("failed to connect to message queue");
                    }
                    return promise.get_future();
                };

                virtual void send(message <buffer_type> &&_buf) const override {

                };

                virtual void close(bool wait_queue=false) {

                };
            };
        }

        template<typename buffer_type>
        using http_endpoint = endpoint_impl<buffer_type>;
    }
}
#endif //INTERPROCESS_SENDER_HPP
