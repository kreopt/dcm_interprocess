#ifndef INTERPROCESS_DEFS_HPP
#define INTERPROCESS_DEFS_HPP

#include <cstdint>
#include <sstream>
#include <binelpro/symbol.hpp>

namespace interproc {
    using block_descriptor_t = uint32_t;
    using byte_t = uint8_t;
    using ibufstream = std::basic_istringstream<byte_t>;
    using obufstream = std::basic_ostringstream<byte_t>;

    const uint32_t BLOCK_DESCRIPTOR_SIZE = sizeof(block_descriptor_t);
    const size_t   MQ_SIZE = 100;
    const size_t   MQ_AMOUNT = 100;

    using namespace bp::literals;

    template <typename msg_type>
    using msg_handler_t = std::function<void(msg_type &&)>;

    enum class protocol : bp::symbol::hash_type {
        ipc = "ipc"_hash,
        tcp = "tcp"_hash
#ifndef WIN32
        ,unix = "unix"_hash
#endif
    };

    inline std::pair<std::string, std::string> parse_endpoint(const std::string &_ep) {
        auto pos = _ep.find("://");
        if (pos == std::string::npos) {
            throw std::runtime_error("invalid protocol. supported types are ipc, tcp and unix");
        }
        return std::make_pair(_ep.substr(0, pos), _ep.substr(pos + 3));
    }
}
#endif //INTERPROCESS_DEFS_HPP
