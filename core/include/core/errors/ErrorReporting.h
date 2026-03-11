#pragma once

#include <exception>

#include "core/errors/IErrorReporter.h"

namespace core::errors {

inline void report(IErrorReporter* reporter, const ErrorEvent& event)
{
    if (reporter) reporter->report(event);
}

inline void reportException(IErrorReporter* reporter,
                            ErrorSeverity severity,
                            const char* origin,
                            std::exception_ptr exception)
{
    if (reporter) reporter->reportException(severity, origin, exception);
}

}
