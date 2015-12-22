#ifndef INTERPROCESS_SENDER_HPP
#define INTERPROCESS_SENDER_HPP

namespace interproc {
    template <typename buffer_type = interproc::buffer >
    class sender {
    public:
        using ptr = std::shared_ptr<sender<buffer_type>>;

        virtual ~sender() {}

        virtual void connect() = 0;

        virtual void send(const buffer_type &_buf) = 0;

        virtual void close() = 0;

        //TODO: on_error function
    };

    using default_sender = sender<>;
}

#endif //INTERPROCESS_SENDER_HPP
