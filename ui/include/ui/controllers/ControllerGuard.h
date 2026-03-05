#pragma once

#include "core/controllers/AppStateController.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <exception>

namespace ui::controllers::guard {

inline bool ensureCore(const AppStateController* core, const char* origin)
{
    if (core) return true;
    core::errors::report(core::errors::ErrorSeverity::Warning,
                         core::errors::codes::GenericError,
                         origin,
                         "AppStateController is null");
    return false;
}

inline void reportException(const char* origin)
{
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::ExceptionError,
                                  origin,
                                  std::current_exception());
}

}