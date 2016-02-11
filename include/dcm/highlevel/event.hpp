#ifndef DCM_DCM_EVENT_LISTENER_HPP
#define DCM_DCM_EVENT_LISTENER_HPP

#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include "../core/listener.hpp"
#include <binelpro/structure.hpp>
#include <memory>

namespace dcm {
    using bp::Log;
    using namespace std::string_literals;

    template <bp::symbol::hash_type serializer_type, typename buffer_type = dcm::buffer,
            class = typename std::enable_if<std::is_convertible<buffer_type, std::string>::value>::type>
    class event {
        using lptr = typename listener<buffer_type>::ptr;
        using sptr = typename session<buffer_type>::ptr;
        typename lptr listener_;
    public:
        explicit event(const lptr &_listener) : listener_(_listener) {
            listener_->on_message.set([](const sptr &_sess, buffer_type &&_buf){
                try {
                    auto event_package = bp::structure::create_from_string<serializer_type>(_buf);
                    auto event = event_package->get("event")->as_symbol();
                    on_event.call(event, event_package->get("data"));
                } catch (bp::structure::structure_error &_e) {
                    Log::e("Failed to parse structure: "s + _e.what());
                }
            });
        }
        handler<std::function<void(const sptr&, bp::structure&&)>>         on_event;
    };

    template <bp::symbol::hash_type serializer_type, typename buffer_type = dcm::buffer,
            class = typename std::enable_if<std::is_convertible<std::string, buffer_type>::value>::type>
    buffer_type make_event(const bp::symbol _evt, const bp::structure &_struct) {
        auto event = bp::structure::create();
        event->emplace({
                               {"event"_sym, _evt},
                               {"data"_dym, _struct}
                       });
        return event->stringify<serializer_type>();
    };
}

#endif //DCM_DCM_EVENT_LISTENER_HPP
