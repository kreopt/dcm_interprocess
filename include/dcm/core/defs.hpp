#ifndef INTERPROCESS_DEFS_HPP
#define INTERPROCESS_DEFS_HPP

#include <cstdint>
#include <sstream>
#include <binelpro/symbol.hpp>

namespace dcm  {
    using block_descriptor_t = uint32_t;
    using byte_t = uint8_t;
    using ibufstream = std::basic_istringstream<byte_t>;
    using obufstream = std::basic_ostringstream<byte_t>;

    const uint32_t BLOCK_DESCRIPTOR_SIZE = sizeof(block_descriptor_t);
    const size_t   MQ_SIZE = 100;
    const size_t   MQ_AMOUNT = 100;
    const size_t QUEUE_SIZE = 1024 * 1024 * 10;

    using namespace bp::literals;
    using namespace std::string_literals;

    template <typename msg_type>
    using msg_handler_t = std::function<void(msg_type &&)>;
    template <typename msg_type>
    using event_matcher_t = std::function<msg_type(const msg_type &)>;


    enum class protocol : bp::symbol::hash_type {
        ipc = "ipc"_hash,
        p2pipc = "p2pipc"_hash,
        tcp = "tcp"_hash,
        http = "http"_hash
    };

    inline std::pair<std::string, std::string> parse_endpoint(const std::string &_ep) {
        auto pos = _ep.find("://");
        if (pos == std::string::npos) {
            throw std::runtime_error("invalid protocol. supported types are ipc, tcp and p2pipc ("s+_ep+")"s);
        }
        return std::make_pair(_ep.substr(0, pos), _ep.substr(pos + 3));
    }
}
#endif //INTERPROCESS_DEFS_HPP
