#pragma once

#include <exception>

#include "core/errors/ErrorEvent.h"

namespace core::errors {

class IErrorReporter {
public:
    virtual ~IErrorReporter() = default;

    virtual void report(const ErrorEvent& event) = 0;
    virtual void reportException(ErrorSeverity severity, const char* origin, std::exception_ptr exception) = 0;
};

}