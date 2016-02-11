#ifndef INTERPROCESS_HTTP_SENDER_HPP
#define INTERPROCESS_HTTP_SENDER_HPP

#include "dcm/core/sender.hpp"
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
            class endpoint_impl : public dcm::sender<buffer_type> {
            public:
                virtual ~endpoint_impl() {
                    close();
                }

                explicit endpoint_impl(const std::string &_endpoint) {
                }

                virtual bool connected() const { return false; };

                virtual void connect() {

                };

                virtual void send(buffer_type &&_buf) const override {

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
