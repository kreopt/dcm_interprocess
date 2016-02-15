#ifndef INTERPROCESS_ENDPOINT_HPP
#define INTERPROCESS_ENDPOINT_HPP

#include <memory>
#include <future>
#include <binelpro/symbol.hpp>
#include <binelpro/os.hpp>
#include "buffer.hpp"
#include "processing_queue.hpp"
#include "exceptions.hpp"
#include "handler.hpp"

namespace dcm  {

    class sender_error : public dcm::exception {
    public:
        explicit sender_error(const char* _reason): dcm::exception(_reason) {}
    };

    namespace {
        using connect_handler_t = std::function<void()>;
    }

    template <typename buffer_type = dcm::buffer >
    class sender {
    public:
        using ptr = std::shared_ptr<sender<buffer_type>>;

        virtual ~sender() {}

        virtual std::string get_endpoint() const = 0;
        virtual bool connected() const = 0;
        virtual void connect() = 0;
        virtual void send(const buffer_type &_buf) const = 0;
        virtual void send(buffer_type &&_buf) const = 0;
//        virtual void multicast_send(const std::string _multicast_id, buffer_type &&_buf) const = 0;
        virtual void close(bool wait_queue=false) = 0;

        handler<connect_handler_t>     on_connect;
        handler<connect_handler_t>     on_disconnect;
    };

    using default_sender = sender<>;
}

#endif //INTERPROCESS_SENDER_HPP
