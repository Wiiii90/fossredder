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

inline void reportException(const char* origin, const char* code = core::errors::codes::ExceptionError)
{
    core::errors::reportException(core::errors::ErrorSeverity::Error,
                                  code,
                                  origin,
                                  std::current_exception());
}

template <typename CorePtr, typename Func>
inline void invokeVoid(CorePtr core, const char* origin, Func&& func)
{
    if (!ensureCore(core, origin)) return;
    try {
        std::invoke(std::forward<Func>(func));
    } catch (const std::exception&) {
        reportException(origin, core::errors::codes::ExceptionStd);
    } catch (...) {
        reportException(origin, core::errors::codes::ExceptionNonStd);
    }
}

template <typename TValue, typename CorePtr, typename Func>
inline TValue invokeValue(CorePtr core, const char* origin, TValue fallback, Func&& func)
{
    if (!ensureCore(core, origin)) return fallback;
    try {
        return std::invoke(std::forward<Func>(func));
    } catch (const std::exception&) {
        reportException(origin, core::errors::codes::ExceptionStd);
    } catch (...) {
        reportException(origin, core::errors::codes::ExceptionNonStd);
    }
    return fallback;
}

}