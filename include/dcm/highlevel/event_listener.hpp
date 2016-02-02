#ifndef DCM_DCM_EVENT_LISTENER_HPP
#define DCM_DCM_EVENT_LISTENER_HPP

#include "../core/defs.hpp"
#include "../core/buffer.hpp"
#include <binelpro/structure.hpp>
#include <memory>

namespace dcm {

    template <bp::symbol::hash_type serializer_type>
    class event_listener : public std::enable_shared_from_this<event_listener<serializer_type>> {
    public:
        using event_handler_t = std::function<void(typename bp::structure::ptr&&)>;
        using ptr = std::shared_ptr<event_listener<serializer_type>>;

        typename event_listener<serializer_type>::ptr on(const bp::symbol &_evt, event_handler_t _handler) {
            listeners_[_evt.hash] = _handler;
            return this->shared_from_this();
        };
    protected:
        void handle_message(dcm::buffer &&_buf) {
            try {
                auto task_info = bp::structure::create_from_string<serializer_type>(_buf);
                auto event = task_info->get("event", "")->as_symbol();
                if (event.valid() && listeners_.count(event.hash)) {
                    auto fn = listeners_.at(event.hash);
                    fn(task_info->get("data", bp::structure::object_t()));
                }
            } catch (bp::structure::structure_error &_e) {
                // log message
                std::cout << _e.what() << std::endl;
            }
        }
    private:
        std::unordered_map<bp::symbol::hash_type, event_handler_t> listeners_;
    };
}

#endif //DCM_DCM_EVENT_LISTENER_HPP
