#ifndef INTERPROCESS_ENDPOINT_HPP
#define INTERPROCESS_ENDPOINT_HPP

#include <memory>
#include <binelpro/symbol.hpp>
#include <binelpro/os.hpp>
#include "buffer.hpp"
#include "processing_queue.hpp"
#include "exceptions.hpp"

namespace interproc {

//    namespace {
        template<typename buffer_type = buffer>
        struct message {
            std::string id;
            buffer_type data;
        };
//    }

    template <typename buffer_type = interproc::buffer >
    class endpoint {
    public:
        using ptr = std::shared_ptr<endpoint<buffer_type>>;

        virtual ~endpoint() {}

        virtual bool connected() const = 0;

        virtual void connect() = 0;

        virtual void send(const message<buffer_type> &_buf) const = 0;

        virtual void close() = 0;


        // TODO: on_error function
        // TODO: on_disconnect function
    };

    using default_endpoint = endpoint<>;
}

#endif //INTERPROCESS_SENDER_HPP
