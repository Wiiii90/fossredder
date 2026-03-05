#pragma once

#include <string>
#include <utility>

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

namespace ui::observability {

inline void reportFlow(core::errors::ErrorSeverity severity,
                       const char* origin,
                       std::string message,
                       core::errors::ErrorContext context = {})
{
    core::errors::report(severity,
                         core::errors::codes::GenericError,
                         origin,
                         std::move(message),
                         std::move(context));
}

}