#pragma once

#include "core/controllers/AppStateController.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <exception>
#include <functional>
#include <utility>

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

template <typename CorePtr, typename Func>
inline void invokeVoid(CorePtr core, const char* origin, Func&& func)
{
    if (!ensureCore(core, origin)) return;
    try {
        std::invoke(std::forward<Func>(func));
    } catch (const std::exception& ex) {
        core::errors::report(core::errors::ErrorSeverity::Error,
                             core::errors::codes::ExceptionStd,
                             origin,
                             ex.what());
        reportException(origin);
    } catch (...) {
        reportException(origin);
    }
}

template <typename TValue, typename CorePtr, typename Func>
inline TValue invokeValue(CorePtr core, const char* origin, TValue fallback, Func&& func)
{
    if (!ensureCore(core, origin)) return fallback;
    try {
        return std::invoke(std::forward<Func>(func));
    } catch (const std::exception& ex) {
        core::errors::report(core::errors::ErrorSeverity::Error,
                             core::errors::codes::ExceptionStd,
                             origin,
                             ex.what());
        reportException(origin);
    } catch (...) {
        reportException(origin);
    }
    return fallback;
}

}