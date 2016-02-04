#ifndef INTERPROCESS_ENDPOINT_HPP
#define INTERPROCESS_ENDPOINT_HPP

#include <memory>
#include <future>
#include <binelpro/symbol.hpp>
#include <binelpro/os.hpp>
#include "buffer.hpp"
#include "processing_queue.hpp"
#include "exceptions.hpp"

namespace dcm  {

//    namespace {
        template<typename buffer_type = buffer>
        struct message {
            std::string id;
            buffer_type data;
        };
//    }

    template <typename buffer_type = dcm::buffer >
    class endpoint {
    public:
        using ptr = std::shared_ptr<endpoint<buffer_type>>;

        virtual ~endpoint() {}

        virtual bool connected() const = 0;

        virtual std::future<bool> connect() = 0;

        virtual void send(message<buffer_type> &&_buf) const = 0;

        virtual void close(bool wait_queue=false) = 0;


        // TODO: on_error function
        std::function<void(void)> on_disconnect;
        std::function<void(void)> on_connect;
    };

    using default_endpoint = endpoint<>;
}

#endif //INTERPROCESS_SENDER_HPP
