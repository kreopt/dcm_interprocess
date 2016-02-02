//
// Created by kreopt on 30.01.16.
//

#ifndef INTERPROCESS_EXCEPTIONS_HPP
#define INTERPROCESS_EXCEPTIONS_HPP

#include <stdexcept>

namespace dcm  {
    class exception : public std::exception {
        std::string reason_;
    public:
        exception(const char* _reason): std::exception(), reason_(_reason) {}

        virtual const char* what() const noexcept override {
            return reason_.c_str();
        }
    };
}

#endif //INTERPROCESS_EXCEPTIONS_HPP
