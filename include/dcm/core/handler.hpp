#ifndef DCM_HANDLER_HPP
#define DCM_HANDLER_HPP

#include <type_traits>
#include <unordered_map>
#include <binelpro/symbol.hpp>

namespace dcm {
    using namespace bp::literals;

    template <typename HandlerType,
            class = typename std::enable_if<std::is_function<HandlerType>::value || std::is_object<HandlerType>::value>::type>
    class handler {
        std::unordered_map<bp::symbol, HandlerType> handlers_;
    public:
        void set_default(const HandlerType &_handler){
            set("default"_sym, _handler);
        }
        void set(const bp::symbol &_name, const HandlerType &_handler) {
            handlers_[_name] = _handler;
        }

        void reset_default() {
            reset("default"_sym);
        }
        void reset(const bp::symbol &_name) {
            handlers_.erase(_name);
        }

        bool empty(){ return handlers_.empty();}

        template <typename... Args>
        void call(Args&&... args) {
            for (auto &h: handlers_) {
                if (h.second) {
                    h.second(std::forward<Args>(args)...);
                }
            }
        }
    };
}

#endif //DCM_HANDLER_HPP
