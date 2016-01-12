#ifndef NET_MESSAGE
#define NET_MESSAGE

#include <deque>
#include <string>
#include <map>
#include <unordered_map>
#include <dcm/interprocess/interproc.hpp>
#include "imessage.hpp"
namespace dcm {
// message class
    class message : dcm::imessage {
    public:
            using block_t = std::unordered_map<std::string, interproc::buffer>;
    private:

        void decode_block(const interproc::buffer &_buf, block_t &_block);

        interproc::buffer encode_block(const block_t &_block) const;

    public:
        typedef std::deque<message> MessageQueue;

        // Constructor
        message();

        message(const interproc::buffer &_encoded);

        message(interproc::buffer &&_encoded);

        message(block_t &&_header, block_t &&_body);

        message(dcm::message &&_message);
        message(const dcm::message &_message) = default;

        void decode_header(const interproc::buffer &_encoded);

        void decode_body(const interproc::buffer &_encoded);

        virtual void decode(const interproc::buffer &_encoded) override;

        virtual interproc::buffer encode() const override;
        interproc::buffer encode_header() const;
        interproc::buffer encode_body() const;

        block_t header;
        block_t body;
    };
}
#endif
